#pragma once

#include <cstdint>

struct Config {
    static constexpr int kScreenWidth = 640;
    static constexpr int kScreenHeight = 480;
    static constexpr int kTargetFPS = 60;

    static constexpr float kGroundY = 368.0F;
    static constexpr float kBirdX = 176.0F;
    static constexpr float kBirdRadius = 15.0F;
    static constexpr float kPipeWidth = 78.0F;
    static constexpr float kPipeGap = 154.0F;
    static constexpr float kPipeSpeed = 174.0F;
    static constexpr float kGravity = 920.0F;
    static constexpr float kFlapVelocity = -330.0F;
    static constexpr float kPipeSpacing = 225.0F;
    static constexpr int kPointsPerLevel = 5;
    static constexpr float kInitialPipeX = 740.0F;
    static constexpr float kPipeSpeedIncrement = 17.4F;
    static constexpr float kMinGap = 120.0F;
    static constexpr float kGapReduction = 3.0F;

    static constexpr int kMedalBronze = 5;
    static constexpr int kMedalSilver = 15;
    static constexpr int kMedalGold = 30;
    static constexpr int kMedalDiamond = 50;
    static constexpr int kMedalPlatinum = 100;

    static constexpr int kMaxParticles = 64;

    static constexpr float kCollisionVibrationStrength = 0.6F;
    static constexpr float kCollisionVibrationDuration = 0.15F;
};
