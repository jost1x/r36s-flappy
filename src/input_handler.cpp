#include "input_handler.h"

#include <raylib.h>

#ifdef R36S_DRM
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#endif

#include <iostream>

InputHandler::InputHandler() = default;

InputHandler::~InputHandler() {
#ifdef R36S_DRM
    for (int fd : rawGamepadFds_) close(fd);
#endif
}

#ifdef R36S_DRM
namespace {
constexpr int kR36sSelectCode = BTN_TRIGGER_HAPPY1;
constexpr int kR36sStartCode = BTN_TRIGGER_HAPPY2;
}  // namespace
#endif

void InputHandler::update() {
#ifdef R36S_DRM
    if (!rawGamepadsInitialized_) initializeRawGamepads();
    pollRawGamepads();
#endif
    gamepadIndex_ = -1;
    for (int index = 0; index < kMaxGamepads; ++index) {
        if (IsGamepadAvailable(index)) {
            gamepadIndex_ = index;
            break;
        }
    }
    if (gamepadIndex_ != lastLoggedGamepadIndex_) {
        std::cerr << "[input] raylib " << (hasGamepad() ? GetGamepadName(gamepadIndex_) : "reports no gamepad") << "\n";
        lastLoggedGamepadIndex_ = gamepadIndex_;
    }

    InputActions next{};
    next.flap = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) ||
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    next.quit = IsKeyPressed(KEY_ESCAPE);
    next.mute = IsKeyPressed(KEY_M);
    next.pause = IsKeyPressed(KEY_P);
    next.options = IsKeyPressed(KEY_O);
    next.up = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    next.down = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
    next.left = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A);
    next.right = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);
    next.confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    next.back = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE);

    if (hasGamepad()) {
        const int gamepad = gamepadIndex_;
        bool start = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT);
        bool select = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT);
        if (start != lastLoggedStart_ || select != lastLoggedSelect_) {
            std::cerr << "[input] raylib START=" << start << " SELECT=" << select << "\n";
            lastLoggedStart_ = start;
            lastLoggedSelect_ = select;
        }
        next.flap = next.flap || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_2);
        next.quit = next.quit || (select && start);
        next.pause = next.pause || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT);
        next.options = next.options || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT);
        next.up = next.up || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP);
        next.down = next.down || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        next.left = next.left || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        next.right = next.right || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        next.confirm = next.confirm || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        next.back = next.back || IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    }
#ifdef R36S_DRM
    next.flap = next.flap || rawActions_.flap;
    next.quit = next.quit || (rawStartPressed_ && rawSelectPressed_);
    next.pause = next.pause || rawActions_.pause;
    next.options = next.options || rawActions_.options;
    next.up = next.up || rawActions_.up;
    next.down = next.down || rawActions_.down;
    next.left = next.left || rawActions_.left;
    next.right = next.right || rawActions_.right;
    next.confirm = next.confirm || rawActions_.confirm;
    next.back = next.back || rawActions_.back;
#endif
    actions_ = next;
}

bool InputHandler::hasGamepad() const { return gamepadIndex_ >= 0; }

bool InputHandler::hasController() const {
#ifdef R36S_DRM
    return hasGamepad() || rawControllerAvailable_;
#else
    return hasGamepad();
#endif
}

int InputHandler::gamepadIndex() const { return gamepadIndex_; }

const InputActions& InputHandler::actions() const { return actions_; }

bool InputHandler::isFullscreenTogglePressed() const { return IsKeyPressed(KEY_F11); }

#ifdef R36S_DRM
void InputHandler::initializeRawGamepads() {
    rawGamepadsInitialized_ = true;
    rawDebugEnabled_ = std::getenv("R36S_INPUT_DEBUG") != nullptr;
    std::cerr << "[input] scanning /dev/input/event* for controller fallback\n";
    DIR* inputDirectory = opendir("/dev/input");
    if (!inputDirectory) return;
    while (dirent* entry = readdir(inputDirectory)) {
        if (strncmp(entry->d_name, "event", 5) != 0) continue;
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;
        char deviceName[256] = "unknown";
        ioctl(fd, EVIOCGNAME(sizeof(deviceName)), deviceName);
        unsigned long keyBits[(KEY_MAX / (sizeof(unsigned long) * 8)) + 1] = {};
        unsigned long eventBits[(EV_MAX / (sizeof(unsigned long) * 8)) + 1] = {};
        auto hasKey = [keyBits](int code) {
            constexpr int bitsPerWord = sizeof(unsigned long) * 8;
            return (keyBits[code / bitsPerWord] >> (code % bitsPerWord)) & 1UL;
        };
        auto hasEvent = [eventBits](int code) {
            constexpr int bitsPerWord = sizeof(unsigned long) * 8;
            return (eventBits[code / bitsPerWord] >> (code % bitsPerWord)) & 1UL;
        };
        if (ioctl(fd, EVIOCGBIT(0, sizeof(eventBits)), eventBits) < 0 ||
            ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0) {
            close(fd);
            continue;
        }
        bool controller = hasEvent(EV_ABS) || std::strstr(deviceName, "Gamepad") != nullptr ||
                          std::strstr(deviceName, "gamepad") != nullptr ||
                          std::strstr(deviceName, "odroidgo3-keys") != nullptr;
        if (!controller) {
            close(fd);
            continue;
        }
        std::cerr << "[input] raw fallback=" << path << " name=\"" << deviceName << "\" A=" << hasKey(BTN_B)
                  << " B=" << hasKey(BTN_A) << " START=" << (hasKey(BTN_START) || hasKey(kR36sStartCode))
                  << " SELECT=" << (hasKey(BTN_SELECT) || hasKey(kR36sSelectCode)) << "\n";
        rawGamepadFds_.push_back(fd);
    }
    closedir(inputDirectory);
    rawControllerAvailable_ = !rawGamepadFds_.empty();
}

void InputHandler::setRawButtonState(int code, bool pressed, bool justPressed) {
    if (code == BTN_START || code == kR36sStartCode) {
        rawStartPressed_ = pressed;
        rawActions_.pause = rawActions_.pause || justPressed;
    } else if (code == BTN_SELECT || code == kR36sSelectCode) {
        rawSelectPressed_ = pressed;
        rawActions_.options = rawActions_.options || justPressed;
    } else if (code == BTN_B) {
        rawActions_.confirm = rawActions_.confirm || justPressed;
        rawActions_.flap = rawActions_.flap || justPressed;
    } else if (code == BTN_A) {
        rawActions_.back = rawActions_.back || justPressed;
        rawActions_.flap = rawActions_.flap || justPressed;
    } else if (code == BTN_DPAD_UP) {
        rawActions_.up = rawActions_.up || justPressed;
    } else if (code == BTN_DPAD_DOWN) {
        rawActions_.down = rawActions_.down || justPressed;
    } else if (code == BTN_DPAD_LEFT) {
        rawActions_.left = rawActions_.left || justPressed;
    } else if (code == BTN_DPAD_RIGHT) {
        rawActions_.right = rawActions_.right || justPressed;
    } else if (code == BTN_TL || code == BTN_TR || code == BTN_TL2 || code == BTN_TR2) {
        rawActions_.flap = rawActions_.flap || justPressed;
    }
}

void InputHandler::pollRawGamepads() {
    rawActions_ = {};
    input_event event{};
    for (int fd : rawGamepadFds_) {
        while (read(fd, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))) {
            if (event.type != EV_KEY) continue;
            setRawButtonState(event.code, event.value != 0, event.value == 1);
            if (rawDebugEnabled_) std::cerr << "[input] raw code=" << event.code << " value=" << event.value << "\n";
        }
    }
}
#endif
