#include "wm.hpp"

#include <algorithm>
#include <ctime>

#include "gfx.hpp"

Desktop::Desktop(int w, int h) : w_(w), h_(h) {
    for (auto& a : appRegistry()) icons_.push_back({a.name, {0, 0, 0, 0}});
    layoutIcons();
}

void Desktop::layoutIcons() {
    int x = 12, y = 12, iw = 76, ih = 60, gap = 8;
    for (auto& ic : icons_) {
        ic.rc = {x, y, iw, ih};
        y += ih + gap;
        if (y + ih > h_ - kTaskbarH) {  // next column
            y = 12;
            x += iw + gap;
        }
    }
}

SDL_Rect Desktop::taskbarRect() const { return {0, h_ - kTaskbarH, w_, kTaskbarH}; }
SDL_Rect Desktop::startButtonRect() const { return {4, h_ - kTaskbarH + 4, 64, kTaskbarH - 8}; }

SDL_Rect Desktop::startMenuRect() const {
    int n = (int)appRegistry().size();
    int itemH = 22, titleW = 180, h = n * itemH + 6;
    SDL_Rect sb = startButtonRect();
    return {sb.x, sb.y - h, titleW, h};
}

int Desktop::startMenuItemAt(int x, int y) const {
    SDL_Rect m = startMenuRect();
    if (x < m.x + 3 || x >= m.x + m.w - 3 || y < m.y + 3 || y >= m.y + m.h - 3) return -1;
    int itemH = 22;
    int idx = (y - (m.y + 3)) / itemH;
    return (idx >= 0 && idx < (int)appRegistry().size()) ? idx : -1;
}

Window* Desktop::topAt(int x, int y) {
    for (auto it = wins_.rbegin(); it != wins_.rend(); ++it)
        if ((*it)->contains(x, y)) return it->get();
    return nullptr;
}

Window* Desktop::focused() const { return wins_.empty() ? nullptr : wins_.back().get(); }

void Desktop::raise(Window* w) {
    for (size_t i = 0; i < wins_.size(); ++i) {
        if (wins_[i].get() == w) {
            auto p = std::move(wins_[i]);
            wins_.erase(wins_.begin() + i);
            wins_.push_back(std::move(p));
            return;
        }
    }
}

void Desktop::launch(const std::string& name) {
    for (auto& a : appRegistry()) {
        if (a.name == name) {
            int x = 60 + cascade_ * 24, y = 40 + cascade_ * 24;
            cascade_ = (cascade_ + 1) % 6;
            wins_.push_back(std::make_unique<Window>(a.make(), x, y));
            return;
        }
    }
}

void Desktop::mouseDown(int x, int y, int button) {
    // Start menu interactions first.
    if (startOpen_) {
        int idx = startMenuItemAt(x, y);
        if (idx >= 0) {
            launch(appRegistry()[idx].name);
            startOpen_ = false;
            return;
        }
        SDL_Rect sb = startButtonRect();
        bool onStart = x >= sb.x && y >= sb.y && x < sb.x + sb.w && y < sb.y + sb.h;
        startOpen_ = false;
        if (onStart) return;  // toggled closed
    }

    SDL_Rect tb = taskbarRect();
    if (y >= tb.y) {
        SDL_Rect sb = startButtonRect();
        if (x >= sb.x && y >= sb.y && x < sb.x + sb.w && y < sb.y + sb.h) {
            startOpen_ = !startOpen_;
            return;
        }
        // Task buttons: raise the matching window.
        int bx = sb.x + sb.w + 6, bw = 150;
        for (auto& wp : wins_) {
            SDL_Rect r = {bx, tb.y + 4, bw, kTaskbarH - 8};
            if (x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h) {
                raise(wp.get());
                return;
            }
            bx += bw + 3;
        }
        return;
    }

    if (Window* w = topAt(x, y)) {
        raise(w);
        if (button == SDL_BUTTON_LEFT && w->hitClose(x, y)) {
            w->closePressed = true;
        } else if (button == SDL_BUTTON_LEFT && w->hitTitle(x, y)) {
            dragging_ = w;
            dragDX_ = x - w->frame().x;
            dragDY_ = y - w->frame().y;
        } else if (w->hitContent(x, y)) {
            SDL_Rect cr = w->contentRect();
            w->app()->onMouse({x - cr.x, y - cr.y, button, true});
        }
        return;
    }

    // Desktop: select / double-click an icon to launch.
    for (size_t i = 0; i < icons_.size(); ++i) {
        SDL_Rect r = icons_[i].rc;
        if (x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h) {
            Uint32 now = SDL_GetTicks();
            if (lastIcon_ == (int)i && now - lastIconClick_ < 400) {
                launch(icons_[i].name);
                lastIcon_ = -1;
            } else {
                lastIcon_ = (int)i;
                lastIconClick_ = now;
            }
            return;
        }
    }
    lastIcon_ = -1;
}

void Desktop::mouseUp(int x, int y, int button) {
    dragging_ = nullptr;
    for (auto& wp : wins_) {
        if (wp->closePressed) {
            wp->closePressed = false;
            if (wp->hitClose(x, y)) wp->wantClose = true;
        }
    }
    if (Window* w = focused(); w && w->hitContent(x, y)) {
        SDL_Rect cr = w->contentRect();
        w->app()->onMouse({x - cr.x, y - cr.y, button, false});
    }
    // Reap closed windows.
    wins_.erase(std::remove_if(wins_.begin(), wins_.end(),
                               [](const std::unique_ptr<Window>& w) { return w->wantClose; }),
                wins_.end());
}

void Desktop::mouseMove(int x, int y) {
    if (dragging_) {
        dragging_->moveTo(x - dragDX_, y - dragDY_);
        return;
    }
    if (Window* w = focused(); w && w->hitContent(x, y)) {
        SDL_Rect cr = w->contentRect();
        w->app()->onMouseMove(x - cr.x, y - cr.y);
    }
}

void Desktop::key(SDL_Keycode k, bool down) {
    if (down && k == SDLK_ESCAPE && startOpen_) {
        startOpen_ = false;
        return;
    }
    if (Window* w = focused()) w->app()->onKey(k, down);
}

void Desktop::update(double dt) {
    for (auto& w : wins_) w->app()->update(dt);
}

void Desktop::drawIcons(Gfx& g) {
    for (size_t i = 0; i < icons_.size(); ++i) {
        SDL_Rect r = icons_[i].rc;
        bool sel = lastIcon_ == (int)i;
        // Simple framed glyph box as the icon.
        SDL_Rect box = {r.x + (r.w - 32) / 2, r.y, 32, 32};
        g.bevel(box, true);
        g.textIn(std::string(1, icons_[i].name[0]), box, c95::navy, Gfx::Center, true);
        SDL_Rect lab = {r.x, r.y + 34, r.w, 16};
        if (sel) g.fill(lab, c95::navy);
        g.textIn(icons_[i].name, lab, sel ? c95::white : c95::white, Gfx::Center);
    }
}

void Desktop::drawTaskbar(Gfx& g) {
    SDL_Rect tb = taskbarRect();
    g.bevel(tb, true);

    SDL_Rect sb = startButtonRect();
    g.bevel(sb, !startOpen_);
    SDL_Rect st = {sb.x + 6 + (startOpen_ ? 1 : 0), sb.y + (startOpen_ ? 1 : 0), sb.w - 6, sb.h};
    g.textIn("Start", st, c95::black, Gfx::Left, true);

    int bx = sb.x + sb.w + 6, bw = 150;
    Window* top = focused();
    for (auto& wp : wins_) {
        SDL_Rect r = {bx, tb.y + 4, bw, kTaskbarH - 8};
        bool active = wp.get() == top;
        g.bevel(r, !active);
        SDL_Rect tr = {r.x + 6 + (active ? 1 : 0), r.y, r.w - 10, r.h};
        g.textIn(wp->app()->title(), tr, c95::black, Gfx::Left, active);
        bx += bw + 3;
        if (bx > tb.w - 90) break;
    }

    // Clock tray.
    SDL_Rect tray = {tb.w - 74, tb.y + 4, 68, kTaskbarH - 8};
    g.thinSunken(tray);
    char buf[16];
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    std::strftime(buf, sizeof buf, "%H:%M", lt);
    g.textIn(buf, tray, c95::black, Gfx::Center);
}

void Desktop::drawStartMenu(Gfx& g) {
    SDL_Rect m = startMenuRect();
    g.bevel(m, true);
    // Left spine (classic blue band).
    SDL_Rect spine = {m.x + 3, m.y + 3, 18, m.h - 6};
    g.fill(spine, c95::navy);
    int itemH = 22;
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int hov = startMenuItemAt(mx, my);
    for (size_t i = 0; i < appRegistry().size(); ++i) {
        SDL_Rect it = {m.x + 24, m.y + 3 + (int)i * itemH, m.w - 27, itemH};
        if ((int)i == hov) g.fill(it, c95::navy);
        SDL_Rect tt = {it.x + 6, it.y, it.w - 6, it.h};
        g.textIn(appRegistry()[i].name, tt, (int)i == hov ? c95::white : c95::black, Gfx::Left);
    }
}

void Desktop::draw(Gfx& g) {
    g.fill({0, 0, w_, h_}, c95::teal);
    drawIcons(g);
    for (auto& w : wins_) w->draw(g, w.get() == focused());
    drawTaskbar(g);
    if (startOpen_) drawStartMenu(g);
}
