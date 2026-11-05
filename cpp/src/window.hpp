#pragma once
// A desktop window: Win95 chrome (border, title bar, close box) wrapping an App
// that draws into the client area.

#include <SDL.h>

#include <memory>

#include "app.hpp"

class Gfx;

class Window {
public:
    static constexpr int kBorder = 3;
    static constexpr int kTitleH = 18;

    Window(std::unique_ptr<App> app, int x, int y);

    App* app() const { return app_.get(); }
    const SDL_Rect& frame() const { return frame_; }
    void moveTo(int x, int y) { frame_.x = x; frame_.y = y; }
    void moveBy(int dx, int dy) { frame_.x += dx; frame_.y += dy; }

    SDL_Rect contentRect() const;
    SDL_Rect titleRect() const;
    SDL_Rect closeRect() const;

    bool contains(int x, int y) const;
    bool hitTitle(int x, int y) const;
    bool hitClose(int x, int y) const;
    bool hitContent(int x, int y) const;

    void draw(Gfx& g, bool focused);

    bool closePressed = false;  // close box held down
    bool wantClose = false;     // set when the user clicks the close box

private:
    std::unique_ptr<App> app_;
    SDL_Rect frame_;
};
