// Memory: flip two cards to find matching pairs. Fewest moves wins.
#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include "../app.hpp"
#include "../gfx.hpp"

namespace {

constexpr int kCols = 4, kRows = 4, kCard = 56, kGap = 8, kHdr = 26;

class Memory : public App {
public:
    Memory() { reset(); }

    const char* title() const override { return "Memory"; }
    void preferredSize(int& w, int& h) const override {
        w = kCols * (kCard + kGap) + kGap;
        h = kRows * (kCard + kGap) + kGap + kHdr;
    }

    void update(double dt) override {
        if (flipBack_ > 0) {
            flipBack_ -= dt;
            if (flipBack_ <= 0) {
                cards_[a_].up = cards_[b_].up = false;
                a_ = b_ = -1;
            }
        }
    }

    void onMouse(const MouseEvent& e) override {
        if (!e.down || e.button != SDL_BUTTON_LEFT) return;
        if (e.y < kHdr) { if (won_) reset(); return; }
        if (flipBack_ > 0) return;
        int cx = (e.x - kGap) / (kCard + kGap), cy = (e.y - kHdr - kGap) / (kCard + kGap);
        if (cx < 0 || cy < 0 || cx >= kCols || cy >= kRows) return;
        int i = cy * kCols + cx;
        Card& c = cards_[i];
        if (c.up || c.matched) return;
        c.up = true;
        if (a_ < 0) { a_ = i; return; }
        b_ = i;
        ++moves_;
        if (cards_[a_].face == cards_[b_].face) {
            cards_[a_].matched = cards_[b_].matched = true;
            a_ = b_ = -1;
            if (std::all_of(cards_.begin(), cards_.end(), [](const Card& c) { return c.matched; }))
                won_ = true;
        } else {
            flipBack_ = 0.7;  // show both briefly, then hide
        }
    }

    void draw(Gfx& g, const SDL_Rect& content) override {
        SDL_Rect hdr = {content.x, content.y, content.w, kHdr};
        g.bevel(hdr, true);
        g.textIn("Moves: " + std::to_string(moves_), {hdr.x + 6, hdr.y, 160, hdr.h}, c95::black,
                 Gfx::Left, true);
        if (won_)
            g.textIn("Solved! (click to play again)", {hdr.x, hdr.y, hdr.w - 6, hdr.h}, c95::navy,
                     Gfx::Right, true);

        for (int i = 0; i < kCols * kRows; ++i) {
            int cx = i % kCols, cy = i / kCols;
            SDL_Rect r = {content.x + kGap + cx * (kCard + kGap),
                          content.y + kHdr + kGap + cy * (kCard + kGap), kCard, kCard};
            const Card& c = cards_[i];
            if (c.matched) {
                g.fill(r, c95::face);
                g.thinSunken(r);
            } else if (c.up) {
                g.bevel(r, true);
                g.textIn(std::string(1, 'A' + c.face), r, faceColor(c.face), Gfx::Center, true);
            } else {
                g.bevel(r, true);
                g.fill({r.x + 6, r.y + 6, r.w - 12, r.h - 12}, c95::navy);
            }
        }
    }

private:
    struct Card { int face; bool up = false, matched = false; };
    std::vector<Card> cards_;
    int a_ = -1, b_ = -1, moves_ = 0;
    double flipBack_ = 0;
    bool won_ = false;
    std::mt19937 rng_{std::random_device{}()};

    SDL_Color faceColor(int f) const {
        static const SDL_Color p[8] = {{200, 0, 0, 255}, {0, 0, 200, 255}, {0, 130, 0, 255},
                                       {160, 0, 160, 255}, {200, 120, 0, 255}, {0, 130, 130, 255},
                                       {90, 90, 90, 255}, {0, 0, 0, 255}};
        return p[f % 8];
    }

    void reset() {
        cards_.clear();
        for (int f = 0; f < kCols * kRows / 2; ++f) {
            cards_.push_back({f});
            cards_.push_back({f});
        }
        std::shuffle(cards_.begin(), cards_.end(), rng_);
        a_ = b_ = -1;
        moves_ = 0;
        flipBack_ = 0;
        won_ = false;
    }
};

}  // namespace

RETRO_REGISTER_APP("Memory", Memory)
