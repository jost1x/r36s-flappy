#include "game/pipe_manager.h"

#include <algorithm>
#include <cstdlib>

namespace {
constexpr float kPipeWidth = 76.0F;
constexpr float kInitialPipeX = 740.0F;
constexpr float kScreenWidth = 640.0F;
constexpr int kMinGapCenter = 177;
constexpr int kMaxGapCenter = 287;
}  // namespace

PipeManager::PipeManager(int pipeCount, float pipeWidth, float pipeGap, float pipeSpeed, float pipeSpacing, float birdX,
                         unsigned int seed)
    : pipeSpeed_(pipeSpeed),
      pipeGap_(pipeGap),
      pipeSpacing_(pipeSpacing),
      birdX_(birdX),
      rng_(seed != 0 ? seed : std::random_device{}()) {
    (void)pipeCount;
    (void)pipeWidth;
    reset();
}

void PipeManager::reset() {
    score_ = 0;
    level_ = 0;
    for (size_t i = 0; i < pipes_.size(); ++i) {
        pipes_[i] = {kInitialPipeX + static_cast<float>(i) * pipeSpacing_,
                     static_cast<float>(kMinGapCenter + (std::rand() % (kMaxGapCenter - kMinGapCenter + 1))), false};
    }
}

void PipeManager::update(float delta, float birdX, float birdRadius, float birdY) {
    (void)birdX;
    (void)birdRadius;
    (void)birdY;

    float currentSpeed = getPipeSpeed();

    for (auto& pipe : pipes_) {
        pipe.x -= currentSpeed * delta;
        if (!pipe.scored && pipe.x + kPipeWidth < birdX_) {
            pipe.scored = true;
            ++score_;
            level_ = score_ / pointsPerLevel_;
        }
        if (pipe.x + kPipeWidth < 0.0F) {
            recyclePipe(pipe);
        }
    }
}

void PipeManager::recyclePipe(Pipe& pipe) {
    float farthestX = pipe.x;
    for (const auto& other : pipes_) farthestX = std::max(farthestX, other.x);
    pipe.x = std::max(farthestX + pipeSpacing_, kScreenWidth + kPipeWidth);
    pipe.gapCenter = static_cast<float>(kMinGapCenter + (std::rand() % (kMaxGapCenter - kMinGapCenter + 1)));
    pipe.scored = false;
}

bool PipeManager::checkCollision(float birdX, float birdRadius, float birdY, float pipeWidth, float pipeGap,
                                 float groundY) const {
    (void)pipeGap;
    (void)groundY;
    float currentGap = getCurrentGap();
    for (const auto& pipe : pipes_) {
        float gapTop = pipe.gapCenter - currentGap / 2.0F;
        float gapBottom = pipe.gapCenter + currentGap / 2.0F;
        bool overlapsPipe = birdX + birdRadius > pipe.x && birdX - birdRadius < pipe.x + pipeWidth;
        if (overlapsPipe && (birdY - birdRadius < gapTop || birdY + birdRadius > gapBottom)) {
            return true;
        }
    }
    return false;
}

void PipeManager::drawAll(float pipeWidth, float pipeGap, float groundY, Color pipeGreen, Color pipeLight,
                          Color pipeDark) const {
    (void)pipeGap;
    float currentGap = getCurrentGap();
    for (const auto& pipe : pipes_) {
        float gapTop = pipe.gapCenter - currentGap / 2.0F;
        float gapBottom = pipe.gapCenter + currentGap / 2.0F;
        Rectangle topBody{pipe.x, 0.0F, pipeWidth, gapTop - 14.0F};
        Rectangle bottomBody{pipe.x, gapBottom + 14.0F, pipeWidth, groundY - gapBottom - 14.0F};
        Rectangle topLip{pipe.x - 7.0F, gapTop - 28.0F, pipeWidth + 14.0F, 28.0F};
        Rectangle bottomLip{pipe.x - 7.0F, gapBottom, pipeWidth + 14.0F, 28.0F};
        for (const Rectangle rectangle : {topBody, bottomBody, topLip, bottomLip}) {
            DrawRectangleRec(rectangle, pipeGreen);
            DrawRectangleLinesEx(rectangle, 3.0F, pipeDark);
            DrawRectangle(static_cast<int>(rectangle.x + 8.0F), static_cast<int>(rectangle.y + 3.0F), 9,
                          static_cast<int>(std::max(0.0F, rectangle.height - 6.0F)), pipeLight);
        }
    }
}
