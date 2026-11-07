# Security review: RetroOS C++/SDL2 version

**Scope:** everything under `cpp/` as of commit `76bc5a3`: `src/*.cpp|hpp`,
`src/apps/*.cpp` (Snake, Minesweeper, Tetris, Memory, Pong, Paint) and
`CMakeLists.txt`. The React/Electron version is out of scope.

**Status:** findings 1, 3 and 4 are fixed in `apps/paint.cpp` (see "Fixes" below).
Findings 2, 5, 6 and 7 are still open.

## Summary

RetroOS is an offline, single-user desktop program with no networking, no IPC, no
privileges and no parsing of its own file formats. That leaves very little attack
surface. The only place where outside data comes in is **Paint**: it opens BMP
files dropped onto the window and writes BMP files to disk. All three real findings
are in those two paths.

The games and the window manager use fixed-size arrays with explicit bounds checks
everywhere they index. A sanitizer build (ASan + UBSan) compiles cleanly with
`-Wall -Wextra` and no warnings.

| # | Severity | Finding | Location | Status |
|---|---|---|---|---|
| 1 | Medium | A tiny crafted BMP makes Paint allocate gigabytes (out-of-memory crash) | `apps/paint.cpp:233` | Fixed |
| 2 | Medium | Untrusted BMP parsing has no minimum SDL version (known SDL BMP CVEs) | `CMakeLists.txt:12`, `apps/paint.cpp:235` | Open |
| 3 | Low | Dropping a FIFO or device file freezes the whole desktop | `apps/paint.cpp:235` | Fixed |
| 4 | Low | Save follows symlinks, overwrites, and uses a predictable name, falling back to the working directory | `apps/paint.cpp:129`, `:587` | Fixed |
| 5 | Low | No upper limit on open windows, and each Paint window keeps up to 33 canvas copies | `wm.cpp:124`, `apps/paint.cpp:498` | Open |
| 6 | Info | Compiler hardening flags depend on the distro's defaults | `CMakeLists.txt` | Open |
| 7 | Info | Unchecked `std::localtime` result | `wm.cpp:299`, `apps/paint.cpp:593` | Open (fixed in `save` only) |

Threat model used: the user is trusted. Files the user is tricked into opening
(downloads, email attachments, shared folders) are not. Other local users on a
shared machine are not.

---

## 1. Crafted BMP causes huge memory allocation (Medium)

`Paint::onDropFile` passes the dropped path straight to `SDL_LoadBMP`, then to
`SDL_ConvertSurfaceFormat`, before it crops to the 494x346 canvas. SDL allocates
a surface of the size the BMP header *claims*, whatever the actual file size.

Reproduced against SDL 2.30.0 (scratch harness that makes the same two calls as
Paint):

| File | Size on disk | Result | Peak RSS |
|---|---|---|---|
| 32 bpp, 30000x30000, no pixel data | 54 bytes | loads **successfully** | **6.8 GB** |
| RLE8, 20000x20000, immediate end-of-bitmap | 1 KB | loads successfully | 1.9 GB |
| 24 bpp, 16000x16000, no pixel data | 54 bytes | fails, but only after allocating | 0.7 GB |

On a typical machine the first file gets RetroOS OOM-killed (or the system starts
swapping hard), and every open window's unsaved work is lost. The file still
"opens" even though it holds no pixels, so this is not just a slow failure path.

**Fix:** before calling `SDL_LoadBMP`, read the 54-byte header yourself (or use
`SDL_RWFromFile` and peek) and reject anything over a sane limit, for example
4096x4096 or `width*height*4 > 64 MB`. Check the file size against the claimed
dimensions for uncompressed BMPs. Only the first 494x346 pixels are used anyway.

## 2. No minimum SDL version for the BMP parser (Medium)

`SDL_LoadBMP` is the only untrusted-input parser in the program, and SDL's BMP and
pixel-format code has a history of memory-safety bugs. For example,
CVE-2021-33657 is a heap overflow reachable via a crafted BMP, fixed in SDL 2.0.20.
`CMakeLists.txt` accepts any `sdl2` that pkg-config finds, so a build against an
old system SDL is silently exposed to those bugs.

**Fix:** pin a floor, e.g. `pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2>=2.30)`,
and keep the size checks from finding 1, which also reduce how much of the parser
is reachable.

## 3. Dropping a FIFO / special file hangs the UI (Low)

The whole desktop is one thread, and `onDropFile` runs inside the event loop.
`SDL_LoadBMP` on a named pipe blocks forever. Reproduced: the harness was still
blocked when the 5-second timeout killed it. Every app freezes and unsaved
Paint work can only be lost by killing the process. Slow network mounts behave the
same way, to a lesser degree.

**Fix:** `stat()` the path first and accept only `S_ISREG` files (and apply the size
limit from finding 1 at the same time). Open with `O_NONBLOCK`, then `fstat` the
opened descriptor instead of the path, so the file can't be swapped between check
and use.

## 4. Unsafe save path: symlinks, overwrite, cwd fallback (Low)

`save()` writes `<dir>/retroos-paint-YYYYMMDD-HHMMSS.bmp` with `SDL_SaveBMP`, which
opens the file with `fopen(path, "wb")`:

- **Symlinks are followed and existing files are truncated.** Reproduced: with
  `link.bmp -> victim.txt`, saving replaced `victim.txt` with BMP data.
- **The name is predictable** (local time, one-second resolution). Two saves in the
  same second silently overwrite the first.
- **If `HOME` is unset or empty, `saveDir()` returns `"."`**, the current working
  directory. If that happens to be a shared, world-writable directory such as `/tmp`,
  another local user can plant `retroos-paint-<upcoming time>.bmp` as a symlink
  to one of the victim's files and get it overwritten on the next Ctrl+S. In the
  normal case (`~/Pictures` or `~`) only the user can create files there, so the
  practical risk is low.

**Fix:** create the file yourself with
`open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644)`, wrap the
fd with `fdopen` + `SDL_RWFromFP`, and use `SDL_SaveBMP_RW`. On `EEXIST`, add a
`-1`, `-2`, … suffix. Fall back to `getpwuid(getuid())->pw_dir` rather than `"."`
when `HOME` is missing.

## 5. Unbounded windows and memory (Low)

`Desktop::launch` never limits how many windows can be open (Start menu,
double-click or `RETRO_AUTOLAUNCH`). Each Paint window holds the canvas plus up to
32 undo snapshots plus redo, about 23 MB, so a long `RETRO_AUTOLAUNCH` list or a lot
of clicking uses memory without limit. This is self-inflicted, so impact is minimal.
A cap of, say, 32 windows fixes it cheaply. Also, the taskbar hit-test in
`mouseDown` doesn't stop where `drawTaskbar` stops drawing (`bx > tb.w - 90`), so
there are clickable task buttons that aren't drawn. That's harmless, but the two
loops should share one layout function.

## 6. Hardening flags left to the distro (Info)

`CMakeLists.txt` sets `-Wall -Wextra` and `Release` but no hardening. Whether the
binary gets PIE, a stack protector, `_FORTIFY_SOURCE` and full RELRO depends on the
toolchain's defaults, and those vary (Arch's GCC defaults to PIE and SSP, but
`_FORTIFY_SOURCE` only comes from makepkg's `CFLAGS`, not a plain `cmake` build).
Suggested additions:

```cmake
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=3 -fstack-clash-protection)
add_link_options(-pie -Wl,-z,relro,-z,now)
option(RETRO_SANITIZE "Build with ASan/UBSan" OFF)
```

## 7. Unchecked `std::localtime` (Info)

`std::localtime` can return `nullptr`, and both `drawTaskbar` and `Paint::save` pass
the result straight to `strftime`, which would dereference it. That can't really
happen with current time values, but it's a one-line guard. Use `localtime_r` and
check its result.

---

## Reviewed and found OK

- **Paint pixel ops:** `plot()` bounds-checks every write. `stamp`, `line`,
  `sprayDot` and `shape` all go through it or clamp to the canvas. `flood()` keeps
  `l`/`r`/`ny` inside the canvas. `at()` is only called with coordinates already
  checked to be on the canvas (`beginStroke` is gated by the canvas hit test).
- **Paint BMP copy loop:** after conversion to ARGB8888 each row has at least `w*4`
  bytes and `w`/`h` are clamped to the canvas, so the copy can't overrun either
  buffer.
- **Paint undo stack:** `pushUndo()` always runs before `drawing_ = true`, so the
  `undo_.back()` uses in `preview()` and the Esc-cancel path never see an empty
  deque (the 32-entry cap pops from the front, after the push).
- **Window manager pointers:** `dragging_` and `captured_` are raw pointers into
  `wins_`. `raise()` moves the `unique_ptr`, not the object, so they stay valid.
  Windows are only removed in `mouseUp`, which clears both pointers first.
- **Games:** Tetris checks `fits()` before every board write, and `lock()`
  re-checks bounds. Minesweeper and Memory range-check the computed cell before
  indexing. Snake's head is checked against the walls before use. Pong just does
  float maths with `dt` capped at 0.1 s.
- **Text rendering:** dropped file names (possibly invalid UTF-8) go to
  `TTF_RenderUTF8_Solid`, which tolerates bad sequences, and they're clipped to the
  status bar. No format-string use anywhere (`fprintf` calls use constant formats).
- **`RETRO_AUTOLAUNCH`:** only matches registered app names, so it can't load code or
  paths.
- **Fonts:** loaded from fixed absolute system paths only, not from the working
  directory or the environment.

## Method and what's left

- Manual read of all 2,187 lines in `cpp/`.
- Built with `-fsanitize=address,undefined` against SDL 2.30.0 / SDL_ttf 2.22.0:
  clean compile, no warnings.
- Findings 1, 3 and 4 reproduced with small standalone harnesses that call the same
  SDL functions the same way Paint does (crafted 54-byte and 1 KB BMPs, a `mkfifo`
  pipe, a symlink).

## Fixes (findings 1, 3, 4)

All in `apps/paint.cpp`:

- **Opening (`loadBmpChecked`)**: the file is opened with `O_NONBLOCK`, `fstat`ed on
  the descriptor, and refused unless it's a regular file under 65 MB. It's then
  read into memory, so the checks and the decoder see the same bytes. The BMP
  header's width and height (OS/2 12-byte and Windows 40+-byte headers, including
  negative top-down heights) must be 1 to 4096, and only then is it handed to
  `SDL_LoadBMP_RW` from memory. Worst-case allocation is now about 64 MB, plus the
  converted copy.
- **Saving (`createNew`, `writeBmp`)**: the file is created with
  `O_CREAT | O_EXCL | O_NOFOLLOW`, so it never follows a symlink and never overwrites.
  If the name is taken, it tries `-1`, `-2`, … up to `-99`. A partial file is removed
  if writing fails. When `HOME` is unset it uses the passwd entry, and if that isn't
  available either it refuses to save rather than writing to the working
  directory. `localtime_r` is checked.

Verified by driving the real `Paint` class (from the app registry, under ASan +
UBSan) with each attack file:

| Input | Before | After |
|---|---|---|
| 54-byte 30000x30000 BMP | 6.8 GB RSS, "opens" | refused, RSS 15 MB |
| 1 KB RLE8 20000x20000 BMP | 1.9 GB RSS | refused |
| named pipe | hangs forever | refused immediately ("not a regular file") |
| `/dev/zero`, non-BMP file | — | refused immediately |
| valid 600x400 BMP | opens, cropped | opens, cropped, pixel-identical to the source crop |
| Ctrl+S with symlinks planted for the next 6 timestamps | victim file overwritten | victim untouched, saved as `…-1.bmp`, `…-2.bmp` |
| Ctrl+S with `HOME` unset | writes to working dir | writes to the passwd home dir |

The full app also ran headless (`SDL_VIDEODRIVER=dummy`) with all six apps open
under the sanitizers, with no reports.

Not done yet:

- Running the full GUI under ASan with scripted input (needs a virtual display),
  and fuzzing the drop path, e.g. libFuzzer over `SDL_LoadBMP_RW` plus the copy
  loop, to look for parser bugs beyond the allocation issue.
- Findings 2, 5, 6 and 7 (the CMake changes, a window cap, and the clock's
  `localtime` guard).
- Not security, but noticed while testing: the font paths in `main.cpp` are
  Arch-only, so on Debian/Ubuntu (`/usr/share/fonts/truetype/dejavu/`) the app
  exits with "no usable font found".
