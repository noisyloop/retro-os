#pragma once
// The interface every RetroOS application/game implements. The window manager owns
// an App, gives it a content rectangle to draw into, and forwards input in
// content-local coordinates (origin at the top-left of the client area).

#include <SDL.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Gfx;

struct MouseEvent {
    int x, y;        // content-local
    int button;      // SDL_BUTTON_LEFT / RIGHT / MIDDLE
    bool down;       // true = press, false = release
};

class App {
public:
    virtual ~App() = default;

    virtual const char* title() const = 0;
    virtual void preferredSize(int& w, int& h) const { w = 320; h = 240; }

    virtual void update(double /*dt*/) {}
    virtual void draw(Gfx& g, const SDL_Rect& content) = 0;

    // Input (only delivered while this app's window is focused). Content-local.
    virtual void onKey(SDL_Keycode /*key*/, bool /*down*/) {}
    virtual void onMouse(const MouseEvent& /*e*/) {}
    // While a button is held over the content, moves and the release keep coming
    // here even outside the client area, so coordinates can be out of range.
    virtual void onMouseMove(int /*x*/, int /*y*/) {}
    // A file dropped onto the RetroOS window while this app is focused.
    virtual void onDropFile(const std::string& /*path*/) {}

    // Optional short status (e.g. score) shown by games; may be empty.
    virtual std::string status() const { return {}; }
};

// Registry: name + factory, so the Start menu and desktop icons can launch apps
// without knowing their types.
struct AppInfo {
    std::string name;
    std::function<std::unique_ptr<App>()> make;
};

std::vector<AppInfo>& appRegistry();
void registerApp(const std::string& name, std::function<std::unique_ptr<App>()> make);

// Convenience macro: register an app at static-init time.
#define RETRO_REGISTER_APP(NAME, TYPE)                                              \
    namespace {                                                                     \
    struct TYPE##_Register {                                                        \
        TYPE##_Register() {                                                         \
            registerApp(NAME, [] { return std::unique_ptr<App>(new TYPE()); });     \
        }                                                                           \
    } TYPE##_register_instance;                                                     \
    }
