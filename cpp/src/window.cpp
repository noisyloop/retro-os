#include "window.hpp"

#include "gfx.hpp"

Window::Window(std::unique_ptr<App> app, int x, int y) : app_(std::move(app)) {
    int cw = 320, ch = 240;
    app_->preferredSize(cw, ch);
    frame_ = {x, y, cw + 2 * kBorder, ch + 2 * kBorder + kTitleH};
}

SDL_Rect Window::contentRect() const {
    return {frame_.x + kBorder, frame_.y + kBorder + kTitleH,
            frame_.w - 2 * kBorder, frame_.h - 2 * kBorder - kTitleH};
}

SDL_Rect Window::titleRect() const {
    return {frame_.x + kBorder, frame_.y + kBorder, frame_.w - 2 * kBorder, kTitleH};
}

SDL_Rect Window::closeRect() const {
    SDL_Rect t = titleRect();
    int s = kTitleH - 4;
    return {t.x + t.w - s - 2, t.y + 2, s, s};
}

bool Window::contains(int x, int y) const {
    return x >= frame_.x && y >= frame_.y && x < frame_.x + frame_.w && y < frame_.y + frame_.h;
}

static bool in(const SDL_Rect& r, int x, int y) {
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

bool Window::hitTitle(int x, int y) const { return in(titleRect(), x, y) && !hitClose(x, y); }
bool Window::hitClose(int x, int y) const { return in(closeRect(), x, y); }
bool Window::hitContent(int x, int y) const { return in(contentRect(), x, y); }

void Window::draw(Gfx& g, bool focused) {
    g.bevel(frame_, /*raised=*/true);

    SDL_Rect tr = titleRect();
    g.fill(tr, focused ? c95::navy : c95::inactiv);
    SDL_Rect tt = {tr.x + 4, tr.y, tr.w - 4, tr.h};
    g.textIn(app_->title(), tt, focused ? c95::white : c95::light, Gfx::Left, /*bold=*/true);

    // Close box with an X.
    SDL_Rect cb = closeRect();
    g.bevel(cb, /*raised=*/!closePressed);
    int cx = cb.x + cb.w / 2, cy = cb.y + cb.h / 2, d = 3;
    SDL_Color xcol = c95::black;
    int off = closePressed ? 1 : 0;
    g.renderer();
    SDL_SetRenderDrawColor(g.renderer(), xcol.r, xcol.g, xcol.b, 255);
    for (int i = -d; i <= d; ++i) {
        SDL_RenderDrawPoint(g.renderer(), cx + i + off, cy + i + off);
        SDL_RenderDrawPoint(g.renderer(), cx - i + off, cy + i + off);
    }

    // Client area: sunken, then let the app paint inside a clip.
    SDL_Rect cr = contentRect();
    g.fill(cr, c95::face);
    g.clip(&cr);
    app_->draw(g, cr);
    g.clip(nullptr);
}
