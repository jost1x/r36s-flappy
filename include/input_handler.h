#pragma once

#include <raylib.h>

#ifdef R36S_DRM
#include <vector>
#endif

class InputHandler {
   public:
    InputHandler();
    ~InputHandler();

    void update();

    bool hasGamepad() const;
    int gamepadIndex() const;
    bool isFlapPressed() const;
    bool isQuitPressed() const;
    bool isMutePressed() const;
    bool isPausePressed() const;
    bool isOptionsPressed() const;
    bool isMenuUpPressed() const;
    bool isMenuDownPressed() const;
    bool isMenuLeftPressed() const;
    bool isMenuRightPressed() const;
    bool isConfirmPressed() const;
    bool isBackPressed() const;
    bool isFullscreenTogglePressed() const;

   private:
#ifdef R36S_DRM
    void initializeRawGamepads();
    void pollRawGamepads();
    std::vector<int> rawGamepadFds_;
    bool rawGamepadsInitialized_ = false;
    bool rawStartPressed_ = false;
    bool rawSelectPressed_ = false;
    bool rawStartPressedEvent_ = false;
    bool rawSelectPressedEvent_ = false;
#endif

    static constexpr int kMaxGamepads = 4;

    int gamepadIndex_ = -1;
    int lastLoggedGamepadIndex_ = -2;
    bool lastLoggedStart_ = false;
    bool lastLoggedSelect_ = false;
};
