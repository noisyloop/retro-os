#pragma once
// Drawing helpers and the Windows 95 palette. Everything is flat rectangles and
// bevels; text is rendered without anti-aliasing for a crisp pixel look.

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

namespace c95 {
constexpr SDL_Color teal    = {0, 128, 128, 255};   // desktop
constexpr SDL_Color face    = {192, 192, 192, 255}; // window / button face
constexpr SDL_Color white   = {255, 255, 255, 255}; // top-left highlight
constexpr SDL_Color light   = {223, 223, 223, 255};
constexpr SDL_Color shadow  = {128, 128, 128, 255}; // bottom-right shadow
constexpr SDL_Color dark    = {0, 0, 0, 255};
constexpr SDL_Color navy    = {0, 0, 128, 255};     // active title bar
constexpr SDL_Color navylo  = {0, 0, 160, 255};
constexpr SDL_Color inactiv = {128, 128, 128, 255}; // inactive title bar
constexpr SDL_Color black   = {0, 0, 0, 255};
}  // namespace c95

class Gfx {
public:
    Gfx(SDL_Renderer* r, TTF_Font* font, TTF_Font* bold) : r_(r), font_(font), bold_(bold) {}

    SDL_Renderer* renderer() const { return r_; }

    void fill(const SDL_Rect& rc, SDL_Color c);
    void rect(const SDL_Rect& rc, SDL_Color c);        // 1px outline
    void hline(int x, int y, int w, SDL_Color c);
    void vline(int x, int y, int h, SDL_Color c);

    // Raised (button/window) or sunken (text field / client area) bevel in the
    // classic two-tone style. Fills the face colour unless fill_face is false.
    void bevel(const SDL_Rect& rc, bool raised, bool fill_face = true);
    void thinSunken(const SDL_Rect& rc);  // 1px sunken groove (status areas)

    enum Align { Left, Center, Right };
    void text(const std::string& s, int x, int y, SDL_Color c, bool bold = false);
    void textIn(const std::string& s, const SDL_Rect& rc, SDL_Color c, Align a = Left,
                bool bold = false);
    int textW(const std::string& s, bool bold = false) const;
    int lineH() const { return TTF_FontHeight(font_); }

    void clip(const SDL_Rect* rc) { SDL_RenderSetClipRect(r_, rc); }

private:
    SDL_Renderer* r_;
    TTF_Font* font_;
    TTF_Font* bold_;
    TTF_Font* pick(bool bold) const { return bold && bold_ ? bold_ : font_; }
};
