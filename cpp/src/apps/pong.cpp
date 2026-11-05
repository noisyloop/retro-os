// Pong: move the mouse to control the left paddle; first to 7 wins.
#include <cmath>
#include <string>

#include "../app.hpp"
#include "../gfx.hpp"

namespace {

constexpr int kW = 360, kH = 240, kPadW = 8, kPadH = 48, kBall = 8, kWin = 7;

class Pong : public App {
public:
    Pong() { reset(); }

    const char* title() const override { return "Pong"; }
    void preferredSize(int& w, int& h) const override { w = kW; h = kH; }

    void onMouseMove(int, int y) override { playerY_ = y - kPadH / 2.f; }

    void update(double dt) override {
        if (pScore_ >= kWin || cScore_ >= kWin) return;
        // Clamp player paddle.
        playerY_ = std::max(0.f, std::min(float(kH - kPadH), playerY_));

        bx_ += vx_ * dt;
        by_ += vy_ * dt;
        if (by_ < 0) { by_ = 0; vy_ = -vy_; }
        if (by_ > kH - kBall) { by_ = kH - kBall; vy_ = -vy_; }

        // CPU tracks the ball with a capped speed.
        float target = by_ - kPadH / 2.f + kBall / 2.f;
        cpuY_ += std::max(-cpuSpeed_ * (float)dt, std::min(cpuSpeed_ * (float)dt, target - cpuY_));
        cpuY_ = std::max(0.f, std::min(float(kH - kPadH), cpuY_));

        // Left paddle collision.
        if (vx_ < 0 && bx_ <= kPadW && by_ + kBall >= playerY_ && by_ <= playerY_ + kPadH) {
            bx_ = kPadW;
            bounce(playerY_);
        }
        // Right paddle collision.
        if (vx_ > 0 && bx_ + kBall >= kW - kPadW && by_ + kBall >= cpuY_ && by_ <= cpuY_ + kPadH) {
            bx_ = kW - kPadW - kBall;
            bounce(cpuY_);
        }
        // Scoring.
        if (bx_ < -kBall) { ++cScore_; serve(1); }
        if (bx_ > kW) { ++pScore_; serve(-1); }
    }

    void draw(Gfx& g, const SDL_Rect& c) override {
        SDL_Rect field = {c.x + 4, c.y + 4, kW - 8, kH - 8};
        g.fill(field, c95::black);
        g.thinSunken({field.x - 2, field.y - 2, field.w + 4, field.h + 4});
        auto rel = [&](int x, int y, int w, int h) {
            g.fill({field.x + x, field.y + y, w, h}, c95::white);
        };
        // Center dashes.
        for (int y = 0; y < field.h; y += 16) rel(field.w / 2 - 1, y, 2, 8);
        rel(2, (int)playerY_, kPadW, kPadH);
        rel(field.w - kPadW - 2, (int)cpuY_, kPadW, kPadH);
        rel((int)bx_, (int)by_, kBall, kBall);

        std::string sc = std::to_string(pScore_) + "   " + std::to_string(cScore_);
        g.textIn(sc, {field.x, field.y + 4, field.w, 20}, c95::white, Gfx::Center, true);
        if (pScore_ >= kWin || cScore_ >= kWin) {
            const char* msg = pScore_ >= kWin ? "You win!  (click to restart)"
                                              : "CPU wins.  (click to restart)";
            g.textIn(msg, {field.x, field.y + field.h / 2 - 10, field.w, 20}, c95::white,
                     Gfx::Center, true);
        }
    }

    void onMouse(const MouseEvent& e) override {
        if (e.down && (pScore_ >= kWin || cScore_ >= kWin)) reset();
    }

private:
    float playerY_ = kH / 2 - kPadH / 2, cpuY_ = kH / 2 - kPadH / 2;
    float bx_ = kW / 2, by_ = kH / 2, vx_ = 150, vy_ = 90;
    float cpuSpeed_ = 160;
    int pScore_ = 0, cScore_ = 0;

    void bounce(float padY) {
        vx_ = -vx_ * 1.05f;                       // speed up a touch each hit
        float hit = (by_ + kBall / 2.f - padY) / kPadH - 0.5f;  // -0.5..0.5
        vy_ = hit * 320.f;
        vx_ = std::max(-480.f, std::min(480.f, vx_));
    }

    void serve(int dir) {
        bx_ = kW / 2.f;
        by_ = kH / 2.f;
        vx_ = 150.f * dir;
        vy_ = 90.f;
    }

    void reset() {
        pScore_ = cScore_ = 0;
        serve(-1);
    }
};

}  // namespace

RETRO_REGISTER_APP("Pong", Pong)
