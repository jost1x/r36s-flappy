#pragma once

#include <raylib.h>

class InputHandler {
   public:
    InputHandler();

    void update();

    bool isFlapPressed() const;
    bool isQuitPressed() const;
    bool isMutePressed() const;
    bool isPausePressed() const;
    bool isOptionsPressed() const;
    bool isFullscreenTogglePressed() const;

   private:
    bool prevSpace_ = false;
    bool prevEnter_ = false;
    bool prevUp_ = false;
    bool prevA_ = false;
    bool prevB_ = false;
    bool prevStart_ = false;
    bool prevEscape_ = false;
    bool prevM_ = false;
    bool prevMiddleRight_ = false;
    bool prevLShoulder_ = false;
    bool prevRShoulder_ = false;
    bool prevLeftClick_ = false;
};
