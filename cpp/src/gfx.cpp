#include "gfx.hpp"

static void set(SDL_Renderer* r, SDL_Color c) { SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a); }

void Gfx::fill(const SDL_Rect& rc, SDL_Color c) {
    set(r_, c);
    SDL_RenderFillRect(r_, &rc);
}

void Gfx::rect(const SDL_Rect& rc, SDL_Color c) {
    set(r_, c);
    SDL_RenderDrawRect(r_, &rc);
}

void Gfx::hline(int x, int y, int w, SDL_Color c) {
    set(r_, c);
    SDL_RenderDrawLine(r_, x, y, x + w - 1, y);
}

void Gfx::vline(int x, int y, int h, SDL_Color c) {
    set(r_, c);
    SDL_RenderDrawLine(r_, x, y, x, y + h - 1);
}

void Gfx::bevel(const SDL_Rect& rc, bool raised, bool fill_face) {
    if (fill_face) fill(rc, c95::face);
    // Two-tone Win95 border: outer + inner edges on each side.
    SDL_Color tl1 = raised ? c95::white : c95::shadow;
    SDL_Color tl2 = raised ? c95::light : c95::dark;
    SDL_Color br1 = raised ? c95::dark : c95::white;
    SDL_Color br2 = raised ? c95::shadow : c95::light;
    const int x = rc.x, y = rc.y, w = rc.w, h = rc.h;
    // Outer frame
    hline(x, y, w, tl1);
    vline(x, y, h, tl1);
    hline(x, y + h - 1, w, br1);
    vline(x + w - 1, y, h, br1);
    // Inner frame
    hline(x + 1, y + 1, w - 2, tl2);
    vline(x + 1, y + 1, h - 2, tl2);
    hline(x + 1, y + h - 2, w - 2, br2);
    vline(x + w - 2, y + 1, h - 2, br2);
}

void Gfx::thinSunken(const SDL_Rect& rc) {
    const int x = rc.x, y = rc.y, w = rc.w, h = rc.h;
    hline(x, y, w, c95::shadow);
    vline(x, y, h, c95::shadow);
    hline(x, y + h - 1, w, c95::white);
    vline(x + w - 1, y, h, c95::white);
}

void Gfx::text(const std::string& s, int x, int y, SDL_Color c, bool bold) {
    if (s.empty()) return;
    SDL_Surface* surf = TTF_RenderUTF8_Solid(pick(bold), s.c_str(), c);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r_, surf);
    SDL_Rect dst{x, y, surf->w, surf->h};
    SDL_FreeSurface(surf);
    if (tex) {
        SDL_RenderCopy(r_, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}

void Gfx::textIn(const std::string& s, const SDL_Rect& rc, SDL_Color c, Align a, bool bold) {
    int w = textW(s, bold), h = TTF_FontHeight(pick(bold));
    int x = rc.x + (a == Left ? 0 : a == Center ? (rc.w - w) / 2 : rc.w - w);
    int y = rc.y + (rc.h - h) / 2;
    text(s, x, y, c, bold);
}

int Gfx::textW(const std::string& s, bool bold) const {
    int w = 0, h = 0;
    TTF_SizeUTF8(pick(bold), s.c_str(), &w, &h);
    return w;
}
