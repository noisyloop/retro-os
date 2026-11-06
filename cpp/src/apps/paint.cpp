// Paint: a small bitmap editor in the classic style. Toolbox on the left, options
// under it, colour palette along the bottom (left click = foreground, right click =
// background), and a status bar. Left-drag draws with the foreground colour,
// right-drag with the background colour. Ctrl+Z/Ctrl+Y undo/redo, Ctrl+N new,
// Ctrl+S saves a .bmp, and dropping a .bmp onto the window opens it.
#include <sys/stat.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "../app.hpp"
#include "../gfx.hpp"

namespace {

// ---- Layout (content-local) ------------------------------------------------
constexpr int kW = 560, kH = 420;
constexpr int kToolX = 4, kToolY = 4, kBtn = 24;               // 2 x 5 tool buttons
constexpr SDL_Rect kOpts = {4, 130, 48, 66};                   // tool options box
constexpr SDL_Rect kFrame = {58, 4, kW - 62, 350};             // sunken canvas frame
constexpr int kCX = kFrame.x + 2, kCY = kFrame.y + 2;          // canvas origin
constexpr int kCW = kFrame.w - 4, kCH = kFrame.h - 4;          // canvas size (pixels)
constexpr SDL_Rect kSwatch = {4, 358, 36, 36};                 // fg/bg preview
constexpr int kPalX = 46, kPalY = 358, kSw = 16, kSwStep = 18, kPalCols = 14;
constexpr int kStatusY = kH - 18;
constexpr size_t kUndoMax = 32;

bool in(const SDL_Rect& r, int x, int y) {
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

// ---- Colours -----------------------------------------------------------------
using Px = Uint32;  // ARGB8888
constexpr Px rgb(int r, int g, int b) { return 0xFF000000u | (Px(r) << 16) | (Px(g) << 8) | Px(b); }
SDL_Color toColor(Px p) { return {Uint8(p >> 16), Uint8(p >> 8), Uint8(p), 255}; }
constexpr Px kWhite = rgb(255, 255, 255), kBlack = rgb(0, 0, 0);

constexpr Px kPalette[2 * kPalCols] = {
    rgb(0, 0, 0),       rgb(128, 128, 128), rgb(128, 0, 0),   rgb(128, 128, 0),
    rgb(0, 128, 0),     rgb(0, 128, 128),   rgb(0, 0, 128),   rgb(128, 0, 128),
    rgb(128, 128, 64),  rgb(0, 64, 64),     rgb(0, 128, 255), rgb(0, 64, 128),
    rgb(128, 0, 255),   rgb(128, 64, 0),
    rgb(255, 255, 255), rgb(192, 192, 192), rgb(255, 0, 0),   rgb(255, 255, 0),
    rgb(0, 255, 0),     rgb(0, 255, 255),   rgb(0, 0, 255),   rgb(255, 0, 255),
    rgb(255, 255, 128), rgb(0, 255, 128),   rgb(128, 255, 255), rgb(128, 128, 255),
    rgb(255, 0, 128),   rgb(255, 128, 64),
};

// ---- Tools -------------------------------------------------------------------
enum Tool { Pencil, Brush, Spray, Eraser, Fill, Pick, Line, Rect, Ellipse, RoundRect, kTools };

struct ToolInfo { const char* name; SDL_Keycode key; const char* hint; };
const ToolInfo kToolInfo[kTools] = {
    {"Pencil", SDLK_p, "Pencil (P): draws a 1-pixel line"},
    {"Brush", SDLK_b, "Brush (B): round brush, pick a size below"},
    {"Airbrush", SDLK_a, "Airbrush (A): sprays while held"},
    {"Eraser", SDLK_e, "Eraser (E): paints the background colour"},
    {"Fill", SDLK_f, "Fill (F): floods an area with colour"},
    {"Pick", SDLK_i, "Pick colour (I): left = foreground, right = background"},
    {"Line", SDLK_l, "Line (L): Shift snaps to 45 degrees"},
    {"Rectangle", SDLK_r, "Rectangle (R): Shift draws a square"},
    {"Ellipse", SDLK_o, "Ellipse (O): Shift draws a circle"},
    {"Rounded rect", SDLK_u, "Rounded rectangle (U): Shift draws a square"},
};

// Sizes offered in the options box for tools that have them.
const int kBrushSizes[3] = {2, 4, 7};
const int kEraserSizes[3] = {6, 10, 16};
const int kSprayRadius[3] = {5, 8, 12};
const int kLineWidths[3] = {1, 2, 4};  // also the outline width for shapes

bool isShape(int t) { return t == Rect || t == Ellipse || t == RoundRect; }
bool hasSizes(int t) { return t == Brush || t == Eraser || t == Spray || t == Line; }

// 16x16 icon bitmaps for the freehand tools (shape tools are drawn procedurally).
const char* const kIcons[6][16] = {
    {   // Pencil
        "................", "............##..", "...........#yy#.", "..........#yy#..",
        ".........#yy#...", "........#yy#....", ".......#yy#.....", "......#yy#......",
        ".....#yy#.......", "....#yy#........", "...#ss#.........", "...#ss#.........",
        "..####..........", "..##............", "................", "................"},
    {   // Brush
        "................", "...........##...", "..........#gg#..", ".........#gg#...",
        "........#gg#....", ".......#gg#.....", "......#gg#......", ".....#ss#.......",
        "....#bbb#.......", "...#bbbb#.......", "...#bbb#........", "..#bbb#.........",
        "..#bb#..........", ".##.............", "................", "................"},
    {   // Airbrush
        "..........#.#...", "...........#.#..", "............#.#.", "....##...#.#....",
        "....##....#.#...", "...####.........", "..#ssss#........", "..#swss#........",
        "..#swss#........", "..#swss#........", "..#ssss#........", "..#ssss#........",
        "..#ssss#........", "..######........", "................", "................"},
    {   // Eraser
        "................", "................", "........#######.", ".......#wwwww##.",
        "......#wwwww#.#.", ".....#wwwww#..#.", "....#wwwww#..#..", "...#######..#...",
        "...#rrrrr#.#....", "...#rrrrr##.....", "...#######......", "................",
        "................", "................", "................", "................"},
    {   // Fill
        "................", ".....##.........", "....#..#........", "....#..##.......",
        "...#####s#......", "..#ssssss#......", ".#ssssssss#.....", "#ssssssss#.bb...",
        ".#ssssss#..bb...", "..#ssss#...bbb..", "...#ss#....bbb..", "....##......b...",
        "................", "................", "................", "................"},
    {   // Pick
        "................", "............##..", "...........####.", "..........#####.",
        "........######..", ".......#.###....", "......#.#.#.....", ".....#.#........",
        "....#.#.........", "...#.#..........", "...##...........", "..#.............",
        "................", "................", "................", "................"},
};

SDL_Color iconColor(char ch) {
    switch (ch) {
    case '#': return c95::black;
    case 'g': return {128, 128, 128, 255};
    case 's': return {192, 192, 192, 255};
    case 'w': return c95::white;
    case 'r': return {200, 40, 40, 255};
    case 'b': return {0, 0, 255, 255};
    case 'y': return {230, 200, 0, 255};
    default: return {0, 0, 0, 0};
    }
}

std::string saveDir() {
    const char* home = std::getenv("HOME");
    if (!home || !*home) return ".";
    std::string pics = std::string(home) + "/Pictures";
    struct stat st;
    if (stat(pics.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return pics;
    return home;
}

std::string baseName(const std::string& p) {
    size_t s = p.find_last_of('/');
    return s == std::string::npos ? p : p.substr(s + 1);
}

// ---- The app -----------------------------------------------------------------
class Paint : public App {
public:
    Paint() : pixels_(size_t(kCW) * kCH, kWhite) {}
    ~Paint() override {
        if (tex_) SDL_DestroyTexture(tex_);
    }

    const char* title() const override { return "Paint"; }
    void preferredSize(int& w, int& h) const override { w = kW; h = kH; }

    // -- Input ---------------------------------------------------------------
    void onMouse(const MouseEvent& e) override {
        mx_ = e.x;
        my_ = e.y;
        if (!e.down) {
            if (drawing_ && e.button == btn_) endStroke();
            return;
        }
        if (drawing_) return;  // a second button while drawing: ignore
        bool left = e.button == SDL_BUTTON_LEFT, right = e.button == SDL_BUTTON_RIGHT;
        if (!left && !right) return;

        if (int t = toolAt(e.x, e.y); t >= 0) {
            if (left) selectTool(t);
            return;
        }
        if (in(kOpts, e.x, e.y)) {
            int row = (e.y - kOpts.y - 2) / 20;
            if (row >= 0 && row < 3 && (hasSizes(tool_) || isShape(tool_)))
                option(tool_) = row;
            return;
        }
        if (int i = paletteAt(e.x, e.y); i >= 0) {
            (left ? fg_ : bg_) = kPalette[i];
            return;
        }
        if (in(kSwatch, e.x, e.y)) {
            std::swap(fg_, bg_);
            return;
        }
        if (in({kCX, kCY, kCW, kCH}, e.x, e.y)) beginStroke(e.button, e.x - kCX, e.y - kCY);
    }

    void onMouseMove(int x, int y) override {
        mx_ = x;
        my_ = y;
        if (!drawing_) return;
        int cx = x - kCX, cy = y - kCY;
        switch (tool_) {
        case Pencil: line(lx_, ly_, cx, cy, 1, ink(), true); break;
        case Brush: line(lx_, ly_, cx, cy, kBrushSizes[opt_[Brush]], ink(), true); break;
        case Eraser: line(lx_, ly_, cx, cy, kEraserSizes[opt_[Eraser]], bg_, false); break;
        case Line:
        case Rect:
        case Ellipse:
        case RoundRect: preview(cx, cy); break;
        default: break;
        }
        lx_ = cx;
        ly_ = cy;
    }

    void onKey(SDL_Keycode k, bool down) override {
        if (!down) return;
        SDL_Keymod mod = SDL_GetModState();
        bool ctrl = mod & KMOD_CTRL, shift = mod & KMOD_SHIFT;
        if (k == SDLK_ESCAPE && drawing_) {  // cancel the stroke in progress
            pixels_ = std::move(undo_.back());
            undo_.pop_back();
            drawing_ = false;
            dirty_ = true;
            return;
        }
        if (drawing_) return;
        if (ctrl) {
            if (k == SDLK_z && !shift) undo();
            else if (k == SDLK_y || (k == SDLK_z && shift)) redo();
            else if (k == SDLK_n) clearCanvas();
            else if (k == SDLK_s) save();
            return;
        }
        if (k == SDLK_x) {
            std::swap(fg_, bg_);
            return;
        }
        for (int t = 0; t < kTools; ++t)
            if (kToolInfo[t].key == k) selectTool(t);
    }

    void onDropFile(const std::string& path) override {
        if (drawing_) return;
        SDL_Surface* raw = SDL_LoadBMP(path.c_str());
        if (!raw) {
            flash("Can't open " + baseName(path) + ": only .bmp files are supported");
            return;
        }
        SDL_Surface* s = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_ARGB8888, 0);
        SDL_FreeSurface(raw);
        if (!s) {
            flash(std::string("Can't read image: ") + SDL_GetError());
            return;
        }
        pushUndo();
        std::fill(pixels_.begin(), pixels_.end(), kWhite);
        int w = std::min(s->w, kCW), h = std::min(s->h, kCH);
        SDL_LockSurface(s);
        for (int y = 0; y < h; ++y) {
            const Px* row = reinterpret_cast<const Px*>(static_cast<const Uint8*>(s->pixels) + y * s->pitch);
            for (int x = 0; x < w; ++x) pixels_[size_t(y) * kCW + x] = row[x] | 0xFF000000u;
        }
        SDL_UnlockSurface(s);
        std::string msg = "Opened " + baseName(path);
        if (s->w > kCW || s->h > kCH)
            msg += " (cropped to " + std::to_string(kCW) + "x" + std::to_string(kCH) + ")";
        SDL_FreeSurface(s);
        dirty_ = true;
        flash(msg);
    }

    void update(double dt) override {
        if (msgTime_ > 0) msgTime_ -= dt;
        if (!drawing_ || tool_ != Spray) return;
        int r = kSprayRadius[opt_[Spray]];
        sprayAcc_ += dt * 45.0 * r;  // dots per second scale with the nozzle size
        while (sprayAcc_ >= 1.0) {
            sprayDot(lx_, ly_, r);
            sprayAcc_ -= 1.0;
        }
    }

    // -- Drawing the UI --------------------------------------------------------
    void draw(Gfx& g, const SDL_Rect& c) override {
        SDL_Renderer* r = g.renderer();
        const int ox = c.x, oy = c.y;

        // Toolbox.
        for (int t = 0; t < kTools; ++t) {
            SDL_Rect b = toolRect(t);
            b.x += ox;
            b.y += oy;
            bool sel = t == tool_;
            g.bevel(b, !sel);
            if (sel) g.fill({b.x + 2, b.y + 2, b.w - 4, b.h - 4}, c95::light);
            drawToolIcon(g, t, b.x + 4 + sel, b.y + 4 + sel);
        }
        drawOptions(g, ox, oy);

        // Canvas.
        g.bevel({ox + kFrame.x, oy + kFrame.y, kFrame.w, kFrame.h}, false, false);
        if (!tex_) {
            tex_ = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kCW, kCH);
            dirty_ = true;
        }
        if (tex_ && dirty_) {
            SDL_UpdateTexture(tex_, nullptr, pixels_.data(), kCW * int(sizeof(Px)));
            dirty_ = false;
        }
        SDL_Rect dst = {ox + kCX, oy + kCY, kCW, kCH};
        if (tex_) SDL_RenderCopy(r, tex_, nullptr, &dst);
        drawCursor(g, c);

        // Colour preview + palette.
        SDL_Rect sw = {ox + kSwatch.x, oy + kSwatch.y, kSwatch.w, kSwatch.h};
        g.bevel(sw, false);
        drawChip(g, {sw.x + 15, sw.y + 15, 16, 16}, bg_);
        drawChip(g, {sw.x + 5, sw.y + 5, 16, 16}, fg_);
        for (int i = 0; i < 2 * kPalCols; ++i) {
            SDL_Rect pr = paletteRect(i);
            pr.x += ox;
            pr.y += oy;
            g.bevel(pr, false, false);
            g.fill({pr.x + 2, pr.y + 2, pr.w - 4, pr.h - 4}, toColor(kPalette[i]));
        }

        // Status bar.
        SDL_Rect st = {ox + 2, oy + kStatusY + 1, kW - 92, 16};
        SDL_Rect co = {ox + kW - 86, oy + kStatusY + 1, 84, 16};
        g.thinSunken(st);
        g.thinSunken(co);
        g.clip(&st);
        g.textIn(statusText(), {st.x + 4, st.y, st.w - 8, st.h}, c95::black);
        SDL_Rect content = c;
        g.clip(&content);
        int cx = mx_ - kCX, cy = my_ - kCY;
        if (drawing_ && (isShape(tool_) || tool_ == Line)) {
            g.textIn(std::to_string(std::abs(cx - sx_) + 1) + "x" + std::to_string(std::abs(cy - sy_) + 1),
                     co, c95::black, Gfx::Center);
        } else if (cx >= 0 && cy >= 0 && cx < kCW && cy < kCH) {
            g.textIn(std::to_string(cx) + ", " + std::to_string(cy), co, c95::black, Gfx::Center);
        }
    }

private:
    std::vector<Px> pixels_;
    std::deque<std::vector<Px>> undo_, redo_;
    SDL_Texture* tex_ = nullptr;
    bool dirty_ = true;

    int tool_ = Pencil, prevTool_ = Pencil;
    int opt_[kTools] = {0, 1, 1, 1, 0, 0, 0};  // size index per tool (Brush, Airbrush, Eraser, Line)
    int shapeStyle_ = 0;                       // 0 outline, 1 outline + fill, 2 solid
    Px fg_ = kBlack, bg_ = kWhite;

    bool drawing_ = false;
    int btn_ = 0;
    int sx_ = 0, sy_ = 0, lx_ = 0, ly_ = 0;  // stroke start / last point (canvas coords)
    int mx_ = -1, my_ = -1;                  // mouse (content coords)
    double sprayAcc_ = 0;
    std::mt19937 rng_{std::random_device{}()};

    std::string msg_;
    double msgTime_ = 0;

    // The options-box row for a tool: a size index, or the shared shape fill style.
    int& option(int t) { return isShape(t) ? shapeStyle_ : opt_[t]; }

    Px ink() const { return btn_ == SDL_BUTTON_RIGHT ? bg_ : fg_; }
    Px paper() const { return btn_ == SDL_BUTTON_RIGHT ? fg_ : bg_; }

    void flash(const std::string& m) {
        msg_ = m;
        msgTime_ = 4.0;
    }

    std::string statusText() const {
        if (msgTime_ > 0) return msg_;
        if (int t = toolAt(mx_, my_); t >= 0) return kToolInfo[t].hint;
        if (paletteAt(mx_, my_) >= 0) return "Left click: foreground colour, right click: background";
        if (in(kSwatch, mx_, my_)) return "Click to swap colours (X)";
        return kToolInfo[tool_].hint;
    }

    // -- Hit testing -----------------------------------------------------------
    static SDL_Rect toolRect(int t) { return {kToolX + (t % 2) * kBtn, kToolY + (t / 2) * kBtn, kBtn, kBtn}; }
    static SDL_Rect paletteRect(int i) {
        return {kPalX + (i % kPalCols) * kSw, kPalY + (i / kPalCols) * kSwStep, kSw, kSw};
    }
    static int toolAt(int x, int y) {
        for (int t = 0; t < kTools; ++t)
            if (in(toolRect(t), x, y)) return t;
        return -1;
    }
    static int paletteAt(int x, int y) {
        for (int i = 0; i < 2 * kPalCols; ++i)
            if (in(paletteRect(i), x, y)) return i;
        return -1;
    }

    void selectTool(int t) {
        if (t == Pick && tool_ != Pick) prevTool_ = tool_;
        tool_ = t;
    }

    // -- Pixel ops -------------------------------------------------------------
    void plot(int x, int y, Px c) {
        if (x < 0 || y < 0 || x >= kCW || y >= kCH) return;
        pixels_[size_t(y) * kCW + x] = c;
    }
    Px at(int x, int y) const { return pixels_[size_t(y) * kCW + x]; }

    void stamp(int x, int y, int d, Px c, bool round) {
        if (d <= 1) {
            plot(x, y, c);
            return;
        }
        int lo = -(d - 1) / 2, hi = d / 2;
        double off = (d % 2 == 0) ? 0.5 : 0.0, rr = (d / 2.0) * (d / 2.0);
        for (int dy = lo; dy <= hi; ++dy)
            for (int dx = lo; dx <= hi; ++dx)
                if (!round || (dx - off) * (dx - off) + (dy - off) * (dy - off) <= rr) plot(x + dx, y + dy, c);
    }

    void line(int x0, int y0, int x1, int y1, int d, Px c, bool round) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            stamp(x0, y0, d, c, round);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        dirty_ = true;
    }

    void sprayDot(int cx, int cy, int r) {
        std::uniform_int_distribution<int> d(-r, r);
        for (int tries = 0; tries < 8; ++tries) {
            int dx = d(rng_), dy = d(rng_);
            if (dx * dx + dy * dy <= r * r) {
                plot(cx + dx, cy + dy, ink());
                dirty_ = true;
                return;
            }
        }
    }

    // Scanline flood fill of the region connected to (x,y) that has its colour.
    void flood(int x, int y, Px c) {
        Px target = at(x, y);
        std::vector<std::pair<int, int>> stack{{x, y}};
        while (!stack.empty()) {
            auto [px, py] = stack.back();
            stack.pop_back();
            if (at(px, py) != target) continue;
            int l = px, r = px;
            while (l > 0 && at(l - 1, py) == target) --l;
            while (r < kCW - 1 && at(r + 1, py) == target) ++r;
            for (int i = l; i <= r; ++i) pixels_[size_t(py) * kCW + i] = c;
            for (int ny : {py - 1, py + 1}) {
                if (ny < 0 || ny >= kCH) continue;
                bool prev = false;
                for (int i = l; i <= r; ++i) {
                    bool m = at(i, ny) == target;
                    if (m && !prev) stack.push_back({i, ny});
                    prev = m;
                }
            }
        }
        dirty_ = true;
    }

    // Rectangle / ellipse / rounded rectangle inside the box (x0,y0)-(x1,y1), inclusive.
    // The outline is everything in the shape but not in the shape shrunk by `t`.
    void shape(int tool, int x0, int y0, int x1, int y1, int style, int t, Px stroke, Px fill) {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        const int R = tool == RoundRect ? std::min({10, (x1 - x0 + 1) / 2, (y1 - y0 + 1) / 2}) : 0;
        auto inside = [&](int x, int y, int ax0, int ay0, int ax1, int ay1, int rad) {
            if (ax0 > ax1 || ay0 > ay1 || x < ax0 || x > ax1 || y < ay0 || y > ay1) return false;
            if (tool == Ellipse) {
                double cx = (ax0 + ax1) / 2.0, cy = (ay0 + ay1) / 2.0;
                double rx = (ax1 - ax0) / 2.0 + 0.5, ry = (ay1 - ay0) / 2.0 + 0.5;
                double nx = (x - cx) / rx, ny = (y - cy) / ry;
                return nx * nx + ny * ny <= 1.0;
            }
            int dx = std::max({0, ax0 + rad - x, x - (ax1 - rad)});
            int dy = std::max({0, ay0 + rad - y, y - (ay1 - rad)});
            return dx * dx + dy * dy <= (rad + 0.5) * (rad + 0.5);
        };
        int cx0 = std::max(x0, 0), cy0 = std::max(y0, 0);
        int cx1 = std::min(x1, kCW - 1), cy1 = std::min(y1, kCH - 1);
        for (int y = cy0; y <= cy1; ++y)
            for (int x = cx0; x <= cx1; ++x) {
                if (!inside(x, y, x0, y0, x1, y1, R)) continue;
                bool inner = inside(x, y, x0 + t, y0 + t, x1 - t, y1 - t, std::max(0, R - t));
                if (!inner || style == 2) plot(x, y, stroke);
                else if (style == 1) plot(x, y, fill);
            }
        dirty_ = true;
    }

    // -- Strokes ---------------------------------------------------------------
    void pushUndo() {
        undo_.push_back(pixels_);
        if (undo_.size() > kUndoMax) undo_.pop_front();
        redo_.clear();
    }
    void undo() {
        if (undo_.empty()) return;
        redo_.push_back(std::move(pixels_));
        pixels_ = std::move(undo_.back());
        undo_.pop_back();
        dirty_ = true;
    }
    void redo() {
        if (redo_.empty()) return;
        undo_.push_back(std::move(pixels_));
        pixels_ = std::move(redo_.back());
        redo_.pop_back();
        dirty_ = true;
    }
    void clearCanvas() {
        pushUndo();
        std::fill(pixels_.begin(), pixels_.end(), kWhite);
        dirty_ = true;
        flash("New picture (Ctrl+Z to undo)");
    }

    void beginStroke(int button, int x, int y) {
        btn_ = button;
        if (tool_ == Pick) {
            (button == SDL_BUTTON_RIGHT ? bg_ : fg_) = at(x, y);
            tool_ = prevTool_;
            return;
        }
        if (tool_ == Fill) {
            if (at(x, y) == ink()) return;  // nothing would change
            pushUndo();
            flood(x, y, ink());
            return;
        }
        pushUndo();
        drawing_ = true;
        sx_ = lx_ = x;
        sy_ = ly_ = y;
        sprayAcc_ = 0;
        switch (tool_) {
        case Pencil: stamp(x, y, 1, ink(), true); break;
        case Brush: stamp(x, y, kBrushSizes[opt_[Brush]], ink(), true); break;
        case Eraser: stamp(x, y, kEraserSizes[opt_[Eraser]], bg_, false); break;
        case Spray:
            for (int i = 0; i < 4; ++i) sprayDot(x, y, kSprayRadius[opt_[Spray]]);
            break;
        default: preview(x, y); break;
        }
        dirty_ = true;
    }

    // Redraw the line/shape being dragged on top of the canvas as it was at press.
    void preview(int x, int y) {
        pixels_ = undo_.back();
        bool shift = SDL_GetModState() & KMOD_SHIFT;
        int dx = x - sx_, dy = y - sy_;
        int w = kLineWidths[opt_[Line]];
        if (tool_ == Line) {
            if (shift) {
                int ax = std::abs(dx), ay = std::abs(dy);
                if (ax > 2 * ay) dy = 0;
                else if (ay > 2 * ax) dx = 0;
                else {
                    int m = std::max(ax, ay);
                    dx = dx < 0 ? -m : m;
                    dy = dy < 0 ? -m : m;
                }
            }
            line(sx_, sy_, sx_ + dx, sy_ + dy, w, ink(), true);
            return;
        }
        if (shift) {
            int m = std::max(std::abs(dx), std::abs(dy));
            dx = dx < 0 ? -m : m;
            dy = dy < 0 ? -m : m;
        }
        shape(tool_, sx_, sy_, sx_ + dx, sy_ + dy, shapeStyle_, w, ink(), paper());
    }

    void endStroke() {
        drawing_ = false;
        dirty_ = true;
    }

    void save() {
        SDL_Surface* s = SDL_CreateRGBSurfaceWithFormatFrom(pixels_.data(), kCW, kCH, 32, kCW * int(sizeof(Px)),
                                                            SDL_PIXELFORMAT_ARGB8888);
        SDL_Surface* rgb24 = s ? SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_BGR24, 0) : nullptr;
        char stamp[32];
        std::time_t now = std::time(nullptr);
        std::strftime(stamp, sizeof stamp, "%Y%m%d-%H%M%S", std::localtime(&now));
        std::string path = saveDir() + "/retroos-paint-" + stamp + ".bmp";
        if (rgb24 && SDL_SaveBMP(rgb24, path.c_str()) == 0) {
            const char* home = std::getenv("HOME");
            std::string shown = path;
            if (home && *home && path.rfind(home, 0) == 0) shown = "~" + path.substr(std::string(home).size());
            flash("Saved " + shown);
        } else {
            flash(std::string("Save failed: ") + SDL_GetError());
        }
        if (rgb24) SDL_FreeSurface(rgb24);
        if (s) SDL_FreeSurface(s);
    }

    // -- UI pieces -------------------------------------------------------------
    static void point(SDL_Renderer* r, int x, int y, SDL_Color c) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
        SDL_RenderDrawPoint(r, x, y);
    }

    void drawToolIcon(Gfx& g, int t, int x, int y) {
        SDL_Renderer* r = g.renderer();
        if (t < 6) {
            for (int row = 0; row < 16; ++row)
                for (int col = 0; kIcons[t][row][col]; ++col) {
                    SDL_Color c = iconColor(kIcons[t][row][col]);
                    if (c.a) point(r, x + col, y + row, c);
                }
            return;
        }
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        switch (t) {
        case Line:
            SDL_RenderDrawLine(r, x + 2, y + 13, x + 13, y + 2);
            SDL_RenderDrawLine(r, x + 3, y + 13, x + 13, y + 3);
            break;
        case Rect: g.rect({x + 1, y + 3, 14, 10}, c95::black); break;
        case Ellipse:
            for (int a = 0; a < 64; ++a) {
                double th = a * 3.14159265 / 32;
                SDL_RenderDrawPoint(r, x + 8 + int(std::lround(6.5 * std::cos(th))) - 1,
                                    y + 8 + int(std::lround(5.0 * std::sin(th))) - 1);
            }
            break;
        case RoundRect:
            g.hline(x + 4, y + 3, 8, c95::black);
            g.hline(x + 4, y + 12, 8, c95::black);
            g.vline(x + 1, y + 6, 4, c95::black);
            g.vline(x + 14, y + 6, 4, c95::black);
            for (int i = 0; i < 2; ++i) {  // 3-pixel corners
                SDL_RenderDrawPoint(r, x + 2 + i, y + 5 - i);
                SDL_RenderDrawPoint(r, x + 13 - i, y + 5 - i);
                SDL_RenderDrawPoint(r, x + 2 + i, y + 10 + i);
                SDL_RenderDrawPoint(r, x + 13 - i, y + 10 + i);
            }
            break;
        default: break;
        }
    }

    void drawOptions(Gfx& g, int ox, int oy) {
        SDL_Rect box = {ox + kOpts.x, oy + kOpts.y, kOpts.w, kOpts.h};
        g.bevel(box, false);
        if (!hasSizes(tool_) && !isShape(tool_)) return;
        SDL_Renderer* r = g.renderer();
        int sel = option(tool_);
        for (int i = 0; i < 3; ++i) {
            SDL_Rect row = {box.x + 3, box.y + 3 + i * 20, box.w - 6, 20};
            bool on = i == sel;
            if (on) g.fill(row, c95::navy);
            SDL_Color ink = on ? c95::white : c95::black;
            int cx = row.x + row.w / 2, cy = row.y + row.h / 2;
            if (tool_ == Line) {
                int w = kLineWidths[i];
                g.fill({row.x + 5, cy - w / 2, row.w - 10, w}, ink);
            } else if (tool_ == Brush || tool_ == Eraser) {
                int d = tool_ == Brush ? kBrushSizes[i] : std::min(kEraserSizes[i], 14);
                int lo = -(d - 1) / 2, hi = d / 2;
                double off = (d % 2 == 0) ? 0.5 : 0.0, rr = (d / 2.0) * (d / 2.0);
                for (int dy = lo; dy <= hi; ++dy)
                    for (int dx = lo; dx <= hi; ++dx)
                        if (tool_ == Eraser || (dx - off) * (dx - off) + (dy - off) * (dy - off) <= rr)
                            point(r, cx + dx, cy + dy, ink);
            } else if (tool_ == Spray) {
                int rad = std::min(kSprayRadius[i], 8);
                for (int k = 0; k < 6 + i * 6; ++k) {  // fixed scatter so it doesn't flicker
                    double th = k * 2.399963, rr = rad * std::sqrt((k + 0.5) / (6 + i * 6));
                    point(r, cx + int(rr * std::cos(th)), cy + int(rr * std::sin(th)), ink);
                }
            } else {  // shape fill styles
                SDL_Rect s = {row.x + 6, row.y + 4, row.w - 12, row.h - 8};
                if (i == 1) g.fill(s, c95::shadow);
                if (i == 2) g.fill(s, ink);
                else g.rect(s, ink);
            }
        }
    }

    void drawChip(Gfx& g, const SDL_Rect& rc, Px c) {
        g.rect(rc, c95::black);
        g.fill({rc.x + 1, rc.y + 1, rc.w - 2, rc.h - 2}, toColor(c));
    }

    // Eraser footprint outline while hovering the canvas, like the classic one.
    void drawCursor(Gfx& g, SDL_Rect content) {
        const int ox = content.x, oy = content.y;
        if (tool_ != Eraser) return;
        int cx = mx_ - kCX, cy = my_ - kCY;
        if (cx < 0 || cy < 0 || cx >= kCW || cy >= kCH) return;
        int d = kEraserSizes[opt_[Eraser]], lo = -(d - 1) / 2;
        SDL_Rect rc = {ox + kCX + cx + lo, oy + kCY + cy + lo, d, d};
        SDL_Rect clip = {ox + kCX, oy + kCY, kCW, kCH};
        g.clip(&clip);
        g.rect(rc, c95::black);
        g.clip(&content);
    }
};

}  // namespace

RETRO_REGISTER_APP("Paint", Paint)
