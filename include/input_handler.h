#pragma once

#include <raylib.h>

#ifdef R36S_DRM
#include <vector>
#endif

struct InputActions {
    bool flap = false;
    bool quit = false;
    bool mute = false;
    bool pause = false;
    bool options = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool confirm = false;
    bool back = false;
};

class InputHandler {
   public:
    InputHandler();
    ~InputHandler();

    void update();

    bool hasGamepad() const;
    bool hasController() const;
    int gamepadIndex() const;
    const InputActions& actions() const;
    bool isFullscreenTogglePressed() const;

   private:
#ifdef R36S_DRM
    void initializeRawGamepads();
    void pollRawGamepads();
    void setRawButtonState(int code, bool pressed, bool justPressed);
    std::vector<int> rawGamepadFds_;
    bool rawGamepadsInitialized_ = false;
    bool rawControllerAvailable_ = false;
    bool rawDebugEnabled_ = false;
    bool rawStartPressed_ = false;
    bool rawSelectPressed_ = false;
    InputActions rawActions_{};
#endif

    static constexpr int kMaxGamepads = 4;

    int gamepadIndex_ = -1;
    int lastLoggedGamepadIndex_ = -2;
    bool lastLoggedStart_ = false;
    bool lastLoggedSelect_ = false;
    InputActions actions_{};
};
