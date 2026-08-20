#pragma once

#include <raylib.h>

class ScreenTransition {
   public:
    enum class Type { None, FadeIn, FadeOut, CrossFade };

    void start(Type type, float duration = 0.5F);
    void update(float delta);
    void drawOverlay(Color color = BLACK) const;
    bool isComplete() const;
    bool isActive() const;
    Type getType() const { return currentType_; }
    float getProgress() const { return progress_; }

   private:
    Type currentType_ = Type::None;
    float progress_ = 0.0F;
    float duration_ = 0.0F;
};
