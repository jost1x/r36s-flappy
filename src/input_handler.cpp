#include "input_handler.h"

#include <raylib.h>

InputHandler::InputHandler() = default;

void InputHandler::update() {
    prevSpace_ = IsKeyDown(KEY_SPACE);
    prevEnter_ = IsKeyDown(KEY_ENTER);
    prevUp_ = IsKeyDown(KEY_UP);
    prevEscape_ = IsKeyDown(KEY_ESCAPE);
    prevM_ = IsKeyDown(KEY_M);
    prevLeftClick_ = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    prevA_ = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    prevB_ = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    prevStart_ = IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    prevMiddleRight_ = IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    prevLShoulder_ = IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
    prevRShoulder_ = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
}

bool InputHandler::isFlapPressed() const {
    return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool InputHandler::isQuitPressed() const {
    return IsKeyPressed(KEY_ESCAPE) ||
           (IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_LEFT) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT));
}

bool InputHandler::isMutePressed() const { return IsKeyPressed(KEY_M); }

bool InputHandler::isPausePressed() const {
    return IsKeyPressed(KEY_P) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
}

bool InputHandler::isOptionsPressed() const {
    return IsKeyPressed(KEY_O) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
}

bool InputHandler::isFullscreenTogglePressed() const { return IsKeyPressed(KEY_F11); }
