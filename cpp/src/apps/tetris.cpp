// Tetris: arrows move/rotate, down soft-drops, space hard-drops.
#include <array>
#include <random>
#include <string>

#include "../app.hpp"
#include "../gfx.hpp"

namespace {

constexpr int kCols = 10, kRows = 20, kCell = 18, kHdr = 24, kSide = 76;

// 7 tetrominoes as 4 rotations of 4 (x,y) offsets, built from a base shape.
struct Shape { int cells[4][2]; SDL_Color color; };
const Shape kShapes[7] = {
    {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}, {0, 200, 200, 255}},   // I
    {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}, {0, 0, 210, 255}},     // J
    {{{2, 0}, {0, 1}, {1, 1}, {2, 1}}, {210, 130, 0, 255}},   // L
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}, {210, 200, 0, 255}},   // O
    {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}, {0, 190, 0, 255}},     // S
    {{{1, 0}, {0, 1}, {1, 1}, {2, 1}}, {160, 0, 180, 255}},   // T
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}, {210, 0, 0, 255}},     // Z
};

class Tetris : public App {
public:
    Tetris() { reset(); }

    const char* title() const override { return "Tetris"; }
    void preferredSize(int& w, int& h) const override {
        w = kCols * kCell + kSide + 12;
        h = kRows * kCell + kHdr + 8;
    }

    void onKey(SDL_Keycode k, bool down) override {
        if (!down) return;
        if (over_) { if (k == SDLK_RETURN || k == SDLK_r) reset(); return; }
        switch (k) {
        case SDLK_LEFT: case SDLK_a:  tryMove(-1, 0); break;
        case SDLK_RIGHT: case SDLK_d: tryMove(1, 0); break;
        case SDLK_DOWN: case SDLK_s:  if (!tryMove(0, 1)) lock(); break;
        case SDLK_UP: case SDLK_w:    tryRotate(); break;
        case SDLK_SPACE:              while (tryMove(0, 1)) {} lock(); break;
        case SDLK_p:                  paused_ = !paused_; break;
        }
    }

    void update(double dt) override {
        if (over_ || paused_) return;
        acc_ += dt;
        double step = std::max(0.08, 0.5 - lines_ * 0.02);
        while (acc_ >= step) {
            acc_ -= step;
            if (!tryMove(0, 1)) lock();
        }
    }

    void draw(Gfx& g, const SDL_Rect& content) override {
        SDL_Rect hdr = {content.x, content.y, content.w, kHdr};
        g.bevel(hdr, true);
        g.textIn("Lines: " + std::to_string(lines_), {hdr.x + 6, hdr.y, 120, hdr.h}, c95::black,
                 Gfx::Left, true);
        g.textIn("Score: " + std::to_string(score_), {hdr.x, hdr.y, hdr.w - 6, hdr.h}, c95::black,
                 Gfx::Right, true);

        SDL_Rect well = {content.x + 4, content.y + kHdr + 4, kCols * kCell, kRows * kCell};
        g.fill(well, c95::black);
        g.thinSunken({well.x - 2, well.y - 2, well.w + 4, well.h + 4});

        for (int y = 0; y < kRows; ++y)
            for (int x = 0; x < kCols; ++x)
                if (board_[y][x] >= 0) block(g, well, x, y, kShapes[board_[y][x]].color);
        int cells[4][2];
        piece(cells);
        for (auto& c : cells)
            if (c[1] >= 0) block(g, well, c[0], c[1], kShapes[cur_].color);

        if (paused_) center(g, well, "PAUSED (P)");
        if (over_) center(g, well, "GAME OVER - Enter");
    }

private:
    int board_[kRows][kCols];
    int cur_ = 0, rot_ = 0, px_ = 0, py_ = 0, next_ = 0;
    int lines_ = 0, score_ = 0;
    bool over_ = false, paused_ = false;
    double acc_ = 0;
    std::mt19937 rng_{std::random_device{}()};

    void block(Gfx& g, const SDL_Rect& well, int x, int y, SDL_Color col) {
        SDL_Rect r = {well.x + x * kCell, well.y + y * kCell, kCell, kCell};
        g.bevel(r, true, false);
        g.fill({r.x + 2, r.y + 2, r.w - 4, r.h - 4}, col);
    }

    void center(Gfx& g, const SDL_Rect& well, const std::string& s) {
        int w = g.textW(s, true) + 16;
        SDL_Rect b = {well.x + (well.w - w) / 2, well.y + well.h / 2 - 12, w, 24};
        g.bevel(b, true);
        g.textIn(s, b, c95::black, Gfx::Center, true);
    }

    // Fill cells[] with the current piece's absolute board coords, applying rotation.
    void piece(int out[4][2]) const { blockCells(cur_, rot_, px_, py_, out); }

    void blockCells(int shape, int rot, int ox, int oy, int out[4][2]) const {
        for (int i = 0; i < 4; ++i) {
            int x = kShapes[shape].cells[i][0], y = kShapes[shape].cells[i][1];
            // Rotate within a 4x4 box: (x,y) -> (y, 3-x) per 90°.
            for (int r = 0; r < (rot & 3); ++r) {
                int nx = y, ny = 3 - x;
                x = nx;
                y = ny;
            }
            out[i][0] = ox + x;
            out[i][1] = oy + y;
        }
    }

    bool fits(int shape, int rot, int ox, int oy) const {
        int c[4][2];
        blockCells(shape, rot, ox, oy, c);
        for (auto& p : c) {
            if (p[0] < 0 || p[0] >= kCols || p[1] >= kRows) return false;
            if (p[1] >= 0 && board_[p[1]][p[0]] >= 0) return false;
        }
        return true;
    }

    bool tryMove(int dx, int dy) {
        if (fits(cur_, rot_, px_ + dx, py_ + dy)) { px_ += dx; py_ += dy; return true; }
        return false;
    }

    void tryRotate() {
        int nr = (rot_ + 1) & 3;
        for (int kick : {0, -1, 1, -2, 2})
            if (fits(cur_, nr, px_ + kick, py_)) { rot_ = nr; px_ += kick; return; }
    }

    void lock() {
        int c[4][2];
        piece(c);
        for (auto& p : c)
            if (p[1] >= 0 && p[1] < kRows && p[0] >= 0 && p[0] < kCols) board_[p[1]][p[0]] = cur_;
        clearLines();
        spawn();
    }

    void clearLines() {
        int cleared = 0;
        for (int y = kRows - 1; y >= 0; --y) {
            bool full = true;
            for (int x = 0; x < kCols; ++x)
                if (board_[y][x] < 0) { full = false; break; }
            if (full) {
                ++cleared;
                for (int yy = y; yy > 0; --yy)
                    for (int x = 0; x < kCols; ++x) board_[yy][x] = board_[yy - 1][x];
                for (int x = 0; x < kCols; ++x) board_[0][x] = -1;
                ++y;  // re-check this row
            }
        }
        if (cleared) {
            lines_ += cleared;
            static const int pts[5] = {0, 40, 100, 300, 1200};
            score_ += pts[cleared];
        }
    }

    void spawn() {
        cur_ = next_;
        next_ = std::uniform_int_distribution<int>(0, 6)(rng_);
        rot_ = 0;
        px_ = 3;
        py_ = -1;
        if (!fits(cur_, rot_, px_, py_)) over_ = true;
    }

    void reset() {
        for (auto& row : board_)
            for (int& v : row) v = -1;
        lines_ = score_ = 0;
        over_ = paused_ = false;
        acc_ = 0;
        next_ = std::uniform_int_distribution<int>(0, 6)(rng_);
        spawn();
    }
};

}  // namespace

RETRO_REGISTER_APP("Tetris", Tetris)
