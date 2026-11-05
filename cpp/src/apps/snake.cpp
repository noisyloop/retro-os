// Snake: the first game, and the one that exercises the shell hardest — real-time
// loop, keyboard input, canvas drawing, score and game-over/restart.
#include <deque>
#include <random>

#include "../app.hpp"
#include "../gfx.hpp"

namespace {

constexpr int kCols = 20, kRows = 15, kCell = 16;

class Snake : public App {
public:
    Snake() { reset(); }

    const char* title() const override { return "Snake"; }
    void preferredSize(int& w, int& h) const override {
        w = kCols * kCell;
        h = kRows * kCell + kBarH;
    }
    std::string status() const override { return "Score: " + std::to_string(score_); }

    void onKey(SDL_Keycode k, bool down) override {
        if (!down) return;
        switch (k) {
        case SDLK_UP: case SDLK_w:    want(0, -1); break;
        case SDLK_DOWN: case SDLK_s:  want(0, 1); break;
        case SDLK_LEFT: case SDLK_a:  want(-1, 0); break;
        case SDLK_RIGHT: case SDLK_d: want(1, 0); break;
        case SDLK_RETURN: case SDLK_r: if (dead_) reset(); break;
        case SDLK_p: if (!dead_) paused_ = !paused_; break;
        }
    }

    void update(double dt) override {
        if (dead_ || paused_) return;
        acc_ += dt;
        double step = 0.14 - std::min(0.08, score_ * 0.003);  // speeds up with score
        while (acc_ >= step) {
            acc_ -= step;
            tick();
        }
    }

    void draw(Gfx& g, const SDL_Rect& content) override {
        // Score bar across the top.
        SDL_Rect bar = {content.x, content.y, content.w, kBarH};
        g.bevel(bar, true);
        g.textIn(status(), {bar.x + 6, bar.y, bar.w - 12, bar.h}, c95::black, Gfx::Left, true);
        g.textIn("Best: " + std::to_string(best_), {bar.x + 6, bar.y, bar.w - 12, bar.h},
                 c95::black, Gfx::Right);

        // Playfield, sunken.
        SDL_Rect pf = {content.x + 4, content.y + kBarH + 4, kCols * kCell, kRows * kCell};
        g.fill(pf, c95::black);
        g.thinSunken({pf.x - 2, pf.y - 2, pf.w + 4, pf.h + 4});

        // Food.
        SDL_Color red = {220, 40, 40, 255};
        cell(g, pf, food_.x, food_.y, red);
        // Snake.
        SDL_Color head = {0, 220, 80, 255}, body = {0, 150, 60, 255};
        for (size_t i = 0; i < snake_.size(); ++i)
            cell(g, pf, snake_[i].x, snake_[i].y, i == 0 ? head : body);

        if (paused_) banner(g, pf, "PAUSED  -  P to resume");
        if (dead_) banner(g, pf, "GAME OVER  -  Enter to play again");
    }

private:
    struct P { int x, y; };

    void cell(Gfx& g, const SDL_Rect& pf, int cx, int cy, SDL_Color col) {
        SDL_Rect r = {pf.x + cx * kCell, pf.y + cy * kCell, kCell, kCell};
        g.fill({r.x + 1, r.y + 1, r.w - 2, r.h - 2}, col);
    }

    void banner(Gfx& g, const SDL_Rect& pf, const std::string& s) {
        int w = g.textW(s, true) + 20, h = 28;
        SDL_Rect b = {pf.x + (pf.w - w) / 2, pf.y + (pf.h - h) / 2, w, h};
        g.bevel(b, true);
        g.textIn(s, b, c95::black, Gfx::Center, true);
    }

    void want(int dx, int dy) {
        if (dx == -dir_.x && dy == -dir_.y && snake_.size() > 1) return;  // no reversing
        next_ = {dx, dy};
    }

    void tick() {
        dir_ = next_;
        P h = {snake_.front().x + dir_.x, snake_.front().y + dir_.y};
        if (h.x < 0 || h.y < 0 || h.x >= kCols || h.y >= kRows) { die(); return; }
        for (auto& s : snake_)
            if (s.x == h.x && s.y == h.y) { die(); return; }
        snake_.push_front(h);
        if (h.x == food_.x && h.y == food_.y) {
            score_ += 1;
            spawnFood();
        } else {
            snake_.pop_back();
        }
    }

    void die() {
        dead_ = true;
        if (score_ > best_) best_ = score_;
    }

    void spawnFood() {
        std::uniform_int_distribution<int> dx(0, kCols - 1), dy(0, kRows - 1);
        for (int tries = 0; tries < 500; ++tries) {
            P p = {dx(rng_), dy(rng_)};
            bool on = false;
            for (auto& s : snake_)
                if (s.x == p.x && s.y == p.y) { on = true; break; }
            if (!on) { food_ = p; return; }
        }
    }

    void reset() {
        snake_.clear();
        snake_.push_back({kCols / 2, kRows / 2});
        snake_.push_back({kCols / 2 - 1, kRows / 2});
        snake_.push_back({kCols / 2 - 2, kRows / 2});
        dir_ = next_ = {1, 0};
        score_ = 0;
        dead_ = paused_ = false;
        acc_ = 0;
        spawnFood();
    }

    static constexpr int kBarH = 22;
    std::deque<P> snake_;
    P dir_{1, 0}, next_{1, 0}, food_{0, 0};
    int score_ = 0, best_ = 0;
    bool dead_ = false, paused_ = false;
    double acc_ = 0;
    std::mt19937 rng_{std::random_device{}()};
};

}  // namespace

RETRO_REGISTER_APP("Snake", Snake)
