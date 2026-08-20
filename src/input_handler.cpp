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
        if (hasGamepad()) {
            std::cerr << "[input] raylib gamepad index=" << gamepadIndex_ << " name=\"" << GetGamepadName(gamepadIndex_)
                      << "\"\n";
        } else {
            std::cerr << "[input] raylib reports no gamepad\n";
        }
        lastLoggedGamepadIndex_ = gamepadIndex_;
    }

    if (!hasGamepad()) return;

    bool raylibStart = IsGamepadButtonDown(gamepadIndex_, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    bool raylibSelect = IsGamepadButtonDown(gamepadIndex_, GAMEPAD_BUTTON_MIDDLE_LEFT);

    if (raylibStart != lastLoggedStart_ || raylibSelect != lastLoggedSelect_) {
        std::cerr << "[input] raylib START=" << raylibStart << " SELECT=" << raylibSelect << "\n";
        lastLoggedStart_ = raylibStart;
        lastLoggedSelect_ = raylibSelect;
    }
}

bool InputHandler::hasGamepad() const { return gamepadIndex_ >= 0; }

int InputHandler::gamepadIndex() const { return gamepadIndex_; }

bool InputHandler::isFlapPressed() const {
    bool keyboard = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    bool mouse = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (!hasGamepad()) return keyboard || mouse;

    bool gamepad = IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_MIDDLE_RIGHT) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_RIGHT_TRIGGER_2) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_LEFT_FACE_UP) ||
                   IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_LEFT_FACE_DOWN);

    return keyboard || mouse || gamepad;
}

bool InputHandler::isQuitPressed() const {
    if (IsKeyPressed(KEY_ESCAPE)) return true;
#ifdef R36S_DRM
    if (rawStartPressed_ && rawSelectPressed_) return true;
#endif
    if (!hasGamepad()) return false;
    return IsGamepadButtonDown(gamepadIndex_, GAMEPAD_BUTTON_MIDDLE_LEFT) &&
           IsGamepadButtonDown(gamepadIndex_, GAMEPAD_BUTTON_MIDDLE_RIGHT);
}

bool InputHandler::isMutePressed() const { return IsKeyPressed(KEY_M); }

bool InputHandler::isPausePressed() const {
    if (IsKeyPressed(KEY_P)) return true;
#ifdef R36S_DRM
    if (rawStartPressedEvent_) return true;
#endif
    if (!hasGamepad()) return false;
    return IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_MIDDLE_RIGHT);
}

bool InputHandler::isOptionsPressed() const {
    if (IsKeyPressed(KEY_O)) return true;
#ifdef R36S_DRM
    if (rawSelectPressedEvent_) return true;
#endif
    if (!hasGamepad()) return false;
    return IsGamepadButtonPressed(gamepadIndex_, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
}

bool InputHandler::isFullscreenTogglePressed() const { return IsKeyPressed(KEY_F11); }

#ifdef R36S_DRM
void InputHandler::initializeRawGamepads() {
    rawGamepadsInitialized_ = true;
    std::cerr << "[input] scanning /dev/input/event* for raw START/SELECT\n";
    DIR* inputDirectory = opendir("/dev/input");
    if (!inputDirectory) {
        std::cerr << "[input] cannot open /dev/input: " << std::strerror(errno) << "\n";
        return;
    }

    while (dirent* entry = readdir(inputDirectory)) {
        if (strncmp(entry->d_name, "event", 5) != 0) continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            std::cerr << "[input] " << path << " open failed: " << std::strerror(errno) << "\n";
            continue;
        }

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
        if (ioctl(fd, EVIOCGBIT(0, sizeof(eventBits)), eventBits) < 0) {
            std::cerr << "[input] " << path << " event capability query failed: " << std::strerror(errno) << "\n";
            close(fd);
            continue;
        }
        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0) {
            std::cerr << "[input] " << path << " capability query failed: " << std::strerror(errno) << "\n";
            close(fd);
            continue;
        }
        bool hasGamepadButton = false;
        for (int code = BTN_JOYSTICK; code < BTN_DIGI; ++code) {
            if (hasKey(code)) {
                hasGamepadButton = true;
                break;
            }
        }
        bool hasGamepadAxes = hasEvent(EV_ABS);
        bool looksLikeGamepad =
            std::strstr(deviceName, "Gamepad") != nullptr || std::strstr(deviceName, "gamepad") != nullptr;
        bool looksLikeR36sKeys = std::strstr(deviceName, "odroidgo3-keys") != nullptr;
        if (!hasGamepadButton && !hasGamepadAxes && !looksLikeGamepad && !looksLikeR36sKeys) {
            std::cerr << "[input] ignoring " << path << " name=\"" << deviceName << "\" (no gamepad buttons or axes)\n";
            close(fd);
            continue;
        }
        std::cerr << "[input] raw controller=" << path << " name=\"" << deviceName << "\""
                  << " BTN_START=" << hasKey(BTN_START) << " BTN_SELECT=" << hasKey(BTN_SELECT)
                  << " EV_ABS=" << hasGamepadAxes << " gamepad_name=" << looksLikeGamepad
                  << " r36s_keys_name=" << looksLikeR36sKeys << "\n";
        rawGamepadFds_.push_back(fd);
    }
    closedir(inputDirectory);
}

void InputHandler::pollRawGamepads() {
    input_event event{};
    rawStartPressedEvent_ = false;
    rawSelectPressedEvent_ = false;
    for (int fd : rawGamepadFds_) {
        while (read(fd, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))) {
            if (event.type != EV_KEY) continue;

            const char* buttonName = "UNKNOWN";
            switch (event.code) {
                case BTN_A:
                    buttonName = "R36S_B (BTN_A)";
                    break;
                case BTN_B:
                    buttonName = "R36S_A (BTN_B)";
                    break;
                case BTN_X:
                    buttonName = "BTN_X";
                    break;
                case BTN_Y:
                    buttonName = "BTN_Y";
                    break;
                case BTN_TL:
                    buttonName = "BTN_TL";
                    break;
                case BTN_TR:
                    buttonName = "BTN_TR";
                    break;
                case BTN_TL2:
                    buttonName = "L2 (BTN_TL2)";
                    break;
                case BTN_TR2:
                    buttonName = "R2 (BTN_TR2)";
                    break;
                case BTN_SELECT:
                    buttonName = "BTN_SELECT";
                    break;
                case BTN_START:
                    buttonName = "BTN_START";
                    break;
                case kR36sSelectCode:
                    buttonName = "R36S_SELECT (704)";
                    break;
                case kR36sStartCode:
                    buttonName = "R36S_START (705)";
                    break;
                case BTN_TRIGGER_HAPPY3:
                    buttonName = "L3 (706)";
                    break;
                case BTN_TRIGGER_HAPPY4:
                    buttonName = "R3 (707)";
                    break;
                case BTN_TRIGGER_HAPPY5:
                    buttonName = "FN (708)";
                    break;
                case BTN_DPAD_UP:
                    buttonName = "BTN_DPAD_UP";
                    break;
                case BTN_DPAD_DOWN:
                    buttonName = "BTN_DPAD_DOWN";
                    break;
                case BTN_DPAD_LEFT:
                    buttonName = "BTN_DPAD_LEFT";
                    break;
                case BTN_DPAD_RIGHT:
                    buttonName = "BTN_DPAD_RIGHT";
                    break;
                default:
                    break;
            }

            std::cerr << "[input] raw button code=" << event.code << " name=" << buttonName << " value=" << event.value
                      << " (1=down, 0=up, 2=repeat)\n";

            bool pressed = event.value != 0;
            if (event.code == BTN_START || event.code == kR36sStartCode) {
                rawStartPressed_ = pressed;
                rawStartPressedEvent_ = event.value == 1;
            }
            if (event.code == BTN_SELECT || event.code == kR36sSelectCode) {
                rawSelectPressed_ = pressed;
                rawSelectPressedEvent_ = event.value == 1;
            }
            std::cerr << "[input] raw event code=" << event.code << " value=" << event.value
                      << " START=" << rawStartPressed_ << " SELECT=" << rawSelectPressed_ << "\n";
        }
    }
}
#endif
