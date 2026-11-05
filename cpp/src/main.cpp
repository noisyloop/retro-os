// RetroOS (C++/SDL2) - a small Windows 95-style desktop with games.
#include <SDL.h>
#include <SDL_ttf.h>

#include <cstdio>

#include "gfx.hpp"
#include "wm.hpp"

static TTF_Font* openFirst(int size, int style) {
    const char* candidates[] = {
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf",
    };
    for (const char* p : candidates) {
        if (TTF_Font* f = TTF_OpenFont(p, size)) {
            if (style) TTF_SetFontStyle(f, style);
            TTF_SetFontHinting(f, TTF_HINTING_MONO);  // crisp, retro
            return f;
        }
    }
    return nullptr;
}

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        return 1;
    }

    int W = 1024, H = 720;
    SDL_Window* win = SDL_CreateWindow("RetroOS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       W, H, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!win || !ren) {
        std::fprintf(stderr, "window/renderer: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Font* font = openFirst(13, 0);
    TTF_Font* bold = openFirst(13, TTF_STYLE_BOLD);
    if (!font) {
        std::fprintf(stderr, "no usable font found\n");
        return 1;
    }

    Gfx gfx(ren, font, bold);
    Desktop desk(W, H);

    // Optional: auto-open apps at startup (comma-separated), handy for testing/screenshots.
    if (const char* al = SDL_getenv("RETRO_AUTOLAUNCH")) {
        std::string s(al), cur;
        for (char ch : s + ",") {
            if (ch == ',') { if (!cur.empty()) desk.launch(cur); cur.clear(); }
            else cur += ch;
        }
    }

    bool running = true;
    Uint32 last = SDL_GetTicks();
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    W = e.window.data1;
                    H = e.window.data2;
                    desk.resize(W, H);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                desk.mouseDown(e.button.x, e.button.y, e.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                desk.mouseUp(e.button.x, e.button.y, e.button.button);
                break;
            case SDL_MOUSEMOTION:
                desk.mouseMove(e.motion.x, e.motion.y);
                break;
            case SDL_KEYDOWN:
                desk.key(e.key.keysym.sym, true);
                break;
            case SDL_KEYUP:
                desk.key(e.key.keysym.sym, false);
                break;
            }
        }

        Uint32 now = SDL_GetTicks();
        double dt = (now - last) / 1000.0;
        if (dt > 0.1) dt = 0.1;
        last = now;
        desk.update(dt);

        desk.draw(gfx);
        SDL_RenderPresent(ren);
    }

    if (bold) TTF_CloseFont(bold);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
