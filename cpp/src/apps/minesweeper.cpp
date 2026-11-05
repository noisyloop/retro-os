// Minesweeper: left-click reveals (flood-filling empty areas), right-click flags.
#include <random>
#include <string>

#include "../app.hpp"
#include "../gfx.hpp"

namespace {

constexpr int kCols = 9, kRows = 9, kMines = 10, kCell = 24, kHdr = 28;

class Minesweeper : public App {
public:
    Minesweeper() { reset(); }

    const char* title() const override { return "Minesweeper"; }
    void preferredSize(int& w, int& h) const override {
        w = kCols * kCell + 8;
        h = kRows * kCell + kHdr + 8;
    }

    void onMouse(const MouseEvent& e) override {
        if (!e.down) return;
        if (over_ || won_) {
            if (e.y < kHdr) reset();  // click the header to restart
            return;
        }
        int cx = (e.x - 4) / kCell, cy = (e.y - kHdr - 4) / kCell;
        if (e.x < 4 || e.y < kHdr + 4 || cx < 0 || cy < 0 || cx >= kCols || cy >= kRows) {
            if (e.y < kHdr) reset();
            return;
        }
        Cell& c = at(cx, cy);
        if (e.button == SDL_BUTTON_RIGHT) {
            if (!c.open) c.flag = !c.flag;
        } else {
            if (!c.flag) reveal(cx, cy);
        }
        checkWin();
    }

    void draw(Gfx& g, const SDL_Rect& content) override {
        // Header: mines-left and a status word.
        SDL_Rect hdr = {content.x + 4, content.y + 4, content.w - 8, kHdr - 8};
        g.bevel(hdr, false);
        int flags = 0;
        for (auto& c : grid_) flags += c.flag;
        g.textIn("Mines: " + std::to_string(kMines - flags), {hdr.x + 6, hdr.y, 120, hdr.h},
                 c95::black, Gfx::Left, true);
        const char* s = won_ ? "You win! (click)" : over_ ? "Boom! (click)" : "";
        g.textIn(s, {hdr.x, hdr.y, hdr.w - 6, hdr.h}, c95::black, Gfx::Right, true);

        for (int y = 0; y < kRows; ++y)
            for (int x = 0; x < kCols; ++x) drawCell(g, content, x, y);
    }

private:
    struct Cell { bool mine = false, open = false, flag = false; int n = 0; };
    Cell grid_[kCols * kRows];
    bool over_ = false, won_ = false, placed_ = false;
    std::mt19937 rng_{std::random_device{}()};

    Cell& at(int x, int y) { return grid_[y * kCols + x]; }

    void reset() {
        for (auto& c : grid_) c = Cell{};
        over_ = won_ = placed_ = false;
    }

    void place(int safeX, int safeY) {
        std::uniform_int_distribution<int> dx(0, kCols - 1), dy(0, kRows - 1);
        int placedN = 0;
        while (placedN < kMines) {
            int x = dx(rng_), y = dy(rng_);
            if (at(x, y).mine || (x == safeX && y == safeY)) continue;
            at(x, y).mine = true;
            ++placedN;
        }
        for (int y = 0; y < kRows; ++y)
            for (int x = 0; x < kCols; ++x) {
                int n = 0;
                for (int dyy = -1; dyy <= 1; ++dyy)
                    for (int dxx = -1; dxx <= 1; ++dxx) {
                        int nx = x + dxx, ny = y + dyy;
                        if (nx >= 0 && ny >= 0 && nx < kCols && ny < kRows && at(nx, ny).mine) ++n;
                    }
                at(x, y).n = n;
            }
        placed_ = true;
    }

    void reveal(int x, int y) {
        if (!placed_) place(x, y);
        Cell& c = at(x, y);
        if (c.open || c.flag) return;
        c.open = true;
        if (c.mine) { over_ = true; return; }
        if (c.n == 0) {
            for (int dyy = -1; dyy <= 1; ++dyy)
                for (int dxx = -1; dxx <= 1; ++dxx) {
                    int nx = x + dxx, ny = y + dyy;
                    if (nx >= 0 && ny >= 0 && nx < kCols && ny < kRows) reveal(nx, ny);
                }
        }
    }

    void checkWin() {
        if (over_) return;
        for (auto& c : grid_)
            if (!c.mine && !c.open) return;
        won_ = true;
    }

    void drawCell(Gfx& g, const SDL_Rect& content, int x, int y) {
        SDL_Rect r = {content.x + 4 + x * kCell, content.y + kHdr + 4 + y * kCell, kCell, kCell};
        Cell& c = at(x, y);
        if (!c.open && !(over_ && c.mine)) {
            g.bevel(r, true);
            if (c.flag) g.textIn("P", r, {200, 0, 0, 255}, Gfx::Center, true);
            return;
        }
        g.fill(r, c95::face);
        g.thinSunken(r);
        if (c.mine) {
            g.fill({r.x + 6, r.y + 6, r.w - 12, r.h - 12}, c95::black);
        } else if (c.n > 0) {
            static const SDL_Color cols[9] = {{0, 0, 0, 255}, {0, 0, 220, 255}, {0, 130, 0, 255},
                                              {220, 0, 0, 255}, {0, 0, 128, 255}, {128, 0, 0, 255},
                                              {0, 128, 128, 255}, {0, 0, 0, 255}, {90, 90, 90, 255}};
            g.textIn(std::to_string(c.n), r, cols[c.n], Gfx::Center, true);
        }
    }
};

}  // namespace

RETRO_REGISTER_APP("Minesweeper", Minesweeper)
