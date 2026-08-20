#include "screen_transition.h"

#include <algorithm>

void ScreenTransition::start(Type type, float duration) {
    currentType_ = type;
    duration_ = duration;
    progress_ = (type == Type::FadeIn) ? 1.0F : 0.0F;
}

void ScreenTransition::update(float delta) {
    if (currentType_ == Type::None) return;

    if (currentType_ == Type::FadeOut) {
        progress_ += delta / duration_;
        if (progress_ >= 1.0F) {
            progress_ = 1.0F;
            currentType_ = Type::None;
        }
    } else if (currentType_ == Type::FadeIn) {
        progress_ -= delta / duration_;
        if (progress_ <= 0.0F) {
            progress_ = 0.0F;
            currentType_ = Type::None;
        }
    }
}

void ScreenTransition::drawOverlay(Color color) const {
    if (currentType_ == Type::None || progress_ <= 0.0F) return;

    unsigned char alpha = static_cast<unsigned char>(progress_ * 255);
    Color overlay = color;
    overlay.a = alpha;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), overlay);
}

bool ScreenTransition::isComplete() const { return currentType_ == Type::None; }

bool ScreenTransition::isActive() const { return currentType_ != Type::None; }
