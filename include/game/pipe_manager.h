#pragma once

#include <raylib.h>

#include <array>
#include <random>

#include "game/pipe.h"

class PipeManager {
   public:
    PipeManager(int pipeCount, float pipeWidth, float pipeGap, float pipeSpeed, float pipeSpacing, float birdX,
                unsigned int seed = 0);

    void reset();
    void update(float delta, float birdX, float birdRadius, float birdY);
    void drawAll(float pipeWidth, float pipeGap, float groundY, Color pipeGreen, Color pipeLight, Color pipeDark) const;

    const std::array<Pipe, 3>& getPipes() const { return pipes_; }

    bool checkCollision(float birdX, float birdRadius, float birdY, float pipeWidth, float pipeGap,
                        float groundY) const;

    int getScore() const { return score_; }
    int getLevel() const { return level_; }
    float getPipeSpeed() const { return pipeSpeed_ + level_ * pipeSpeedIncrement_; }
    float getCurrentGap() const { return std::max(minGap_, pipeGap_ - level_ * gapReduction_); }

   private:
    void recyclePipe(Pipe& pipe);

    std::array<Pipe, 3> pipes_;
    int score_ = 0;
    int level_ = 0;
    float pipeSpeed_;
    float pipeSpeedIncrement_ = 17.4F;
    float pipeGap_;
    float minGap_ = 120.0F;
    float gapReduction_ = 3.0F;
    float pipeSpacing_;
    float birdX_;
    int pointsPerLevel_ = 5;
    std::mt19937 rng_;
};
