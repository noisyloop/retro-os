#pragma once
// The desktop / window manager: owns the windows (back-to-front z-order), draws the
// teal desktop, desktop icons, taskbar and Start menu, and routes all input.

#include <SDL.h>

#include <memory>
#include <string>
#include <vector>

#include "window.hpp"

class Gfx;

class Desktop {
public:
    static constexpr int kTaskbarH = 28;

    Desktop(int w, int h);

    void resize(int w, int h) { w_ = w; h_ = h; }

    void mouseDown(int x, int y, int button);
    void mouseUp(int x, int y, int button);
    void mouseMove(int x, int y);
    void key(SDL_Keycode k, bool down);
    void dropFile(const std::string& path);

    void update(double dt);
    void draw(Gfx& g);

    void launch(const std::string& name);

private:
    struct Icon { std::string name; SDL_Rect rc; };

    Window* topAt(int x, int y);          // topmost window containing (x,y)
    void raise(Window* w);                // move to front (focus)
    Window* focused() const;              // topmost window, or null
    SDL_Rect startButtonRect() const;
    SDL_Rect taskbarRect() const;
    SDL_Rect startMenuRect() const;
    int startMenuItemAt(int x, int y) const;  // -1 if none
    void layoutIcons();
    void drawTaskbar(Gfx& g);
    void drawStartMenu(Gfx& g);
    void drawIcons(Gfx& g);

    int w_, h_;
    std::vector<std::unique_ptr<Window>> wins_;  // front() = bottom, back() = top
    std::vector<Icon> icons_;

    // Interaction state.
    Window* dragging_ = nullptr;
    Window* captured_ = nullptr;  // window whose content got the last press
    int dragDX_ = 0, dragDY_ = 0;
    bool startOpen_ = false;
    int cascade_ = 0;

    int lastIcon_ = -1;
    Uint32 lastIconClick_ = 0;
};
