#include "game/bird.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kMaxRotation = 70.0F;
constexpr float kRotationSpeed = 4.0F;
constexpr float kWingSpeed = 12.0F;
constexpr float kFlapWingOffset = -6.0F;
constexpr float kNeutralWingOffset = 0.0F;
}  // namespace

Bird::Bird(float x, float startY, float radius)
    : x_(x),
      startY_(startY),
      y_(startY),
      velocity_(0.0F),
      radius_(radius),
      rotation_(0.0F),
      wingOffset_(0.0F),
      wingPhase_(0.0F) {}

void Bird::reset() {
    y_ = startY_;
    velocity_ = 0.0F;
    rotation_ = 0.0F;
    wingOffset_ = 0.0F;
    wingPhase_ = 0.0F;
}

void Bird::flap(float velocity) {
    velocity_ = velocity;
    wingOffset_ = kFlapWingOffset;
}

void Bird::update(float gravity, float delta) {
    velocity_ += gravity * delta;
    y_ += velocity_ * delta;

    float targetRotation = 0.0F;
    if (velocity_ < 0) {
        targetRotation = -30.0F;
    } else if (velocity_ > 200.0F) {
        targetRotation = std::min(kMaxRotation, (velocity_ - 200.0F) * 0.3F);
    }
    rotation_ += (targetRotation - rotation_) * kRotationSpeed * delta;

    wingPhase_ += kWingSpeed * delta;
    if (wingOffset_ < kNeutralWingOffset) {
        wingOffset_ += (kNeutralWingOffset - wingOffset_) * 8.0F * delta;
    } else {
        wingOffset_ = std::sin(wingPhase_) * 3.0F;
    }
}

void Bird::draw() const {
    Vector2 center{x_, y_};

    DrawCircleV({center.x + 2.0F, center.y + 3.0F}, radius_ + 2.0F, Color{165, 84, 29, 170});

    DrawCircleV(center, radius_, Color{255, 220, 45, 255});

    DrawCircleV({center.x - 7.0F, center.y + 5.0F + wingOffset_}, 8.0F, Color{245, 157, 31, 255});

    DrawCircleV({center.x + 6.0F, center.y - 6.0F}, 5.5F, RAYWHITE);
    DrawCircleV({center.x + 8.0F, center.y - 6.0F}, 2.4F, Color{31, 61, 86, 255});

    DrawTriangle({center.x + 13.0F, center.y + 2.0F}, {center.x + 27.0F, center.y + 7.0F},
                 {center.x + 13.0F, center.y + 12.0F}, Color{245, 107, 34, 255});

    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius_, Color{31, 61, 86, 255});
}
