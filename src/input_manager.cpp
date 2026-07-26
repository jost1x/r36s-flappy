#include "../include/input_manager.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdlib>

InputManager::InputManager() {}

InputManager::~InputManager() { close(); }

bool InputManager::open() {
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        spdlog::error("Failed to init game controller: {}", SDL_GetError());
        return false;
    }

    // Try to open first connected controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                spdlog::info("Game controller opened: {}", SDL_GameControllerName(controller));
                initialized = true;
                return true;
            }
        }
    }

    spdlog::info("No game controller found, using keyboard");
    initialized = true;
    return true;
}

void InputManager::close() {
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    initialized = false;
}

Action InputManager::handleEvent(SDL_Event* event) {
    if (!event) return Action::None;

    switch (event->type) {
        case SDL_KEYDOWN:
            return handleKeyboardEvent(event->key);
        case SDL_CONTROLLERBUTTONDOWN:
            return handleControllerButtonEvent(event->cbutton, true);
        case SDL_CONTROLLERBUTTONUP:
            return handleControllerButtonEvent(event->cbutton, false);
        case SDL_CONTROLLERAXISMOTION:
            return handleControllerAxisEvent(event->caxis);
        default:
            return Action::None;
    }
}

bool InputManager::shouldExit(const SDL_Event& event) {
    if (event.type == SDL_QUIT) return true;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return true;

    if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
        const bool pressed = event.type == SDL_CONTROLLERBUTTONDOWN;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) startPressed_ = pressed;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) backPressed_ = pressed;
    }
    if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
        const bool pressed = event.type == SDL_JOYBUTTONDOWN;
        if (event.jbutton.button == 13) startPressed_ = pressed;
        if (event.jbutton.button == 12) backPressed_ = pressed;
    }
    return startPressed_ && backPressed_;
}

Action InputManager::handleKeyboardEvent(const SDL_KeyboardEvent& key) {
    if (key.state != SDL_PRESSED) return Action::None;

    switch (key.keysym.sym) {
        case SDLK_UP:
            return Action::Up;
        case SDLK_DOWN:
            return Action::Down;
        case SDLK_LEFT:
            return Action::Left;
        case SDLK_RIGHT:
            return Action::Right;
        case SDLK_RETURN:
        case SDLK_SPACE:
            return Action::Confirm;
        case SDLK_BACKSPACE:
            return Action::Back;
        case SDLK_r:
            return Action::Refresh;
        case SDLK_f:
            return Action::Favorite;
        case SDLK_q:
            return Action::MoveFavoriteUp;
        case SDLK_e:
            return Action::MoveFavoriteDown;
        case SDLK_ESCAPE:
            return Action::Exit;
        default:
            return Action::None;
    }
}

Action InputManager::handleControllerButtonEvent(const SDL_ControllerButtonEvent& btn, bool pressed) {
    if (!pressed) return Action::None;

    switch (btn.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return Action::Up;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return Action::Down;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return Action::Left;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return Action::Right;
        // The R36S uses the Nintendo-style face-button layout: its physical
        // A button is reported by SDL as BUTTON_B and its B as BUTTON_A.
        case SDL_CONTROLLER_BUTTON_B:
        case SDL_CONTROLLER_BUTTON_START:
            return Action::Confirm;
        case SDL_CONTROLLER_BUTTON_A:
        case SDL_CONTROLLER_BUTTON_BACK:
            return Action::Back;
        case SDL_CONTROLLER_BUTTON_Y:
            return Action::Refresh;
        case SDL_CONTROLLER_BUTTON_X:
            return Action::Favorite;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return Action::MoveFavoriteUp;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return Action::MoveFavoriteDown;
        default:
            return Action::None;
    }
}

Action InputManager::handleControllerAxisEvent(const SDL_ControllerAxisEvent& axis) {
    constexpr Sint16 kEngageThreshold = 26000;
    constexpr Sint16 kReleaseThreshold = 16000;
    constexpr Uint64 kInitialRepeatDelayMs = 320;
    constexpr Uint64 kRepeatIntervalMs = 140;

    const bool leftStick = axis.axis == SDL_CONTROLLER_AXIS_LEFTX || axis.axis == SDL_CONTROLLER_AXIS_LEFTY;
    const bool rightStick = axis.axis == SDL_CONTROLLER_AXIS_RIGHTX || axis.axis == SDL_CONTROLLER_AXIS_RIGHTY;
    if ((!leftStick && !rightStick) || !controller) return Action::None;

    const size_t stickIndex = leftStick ? 0 : 1;
    const auto horizontalAxis = leftStick ? SDL_CONTROLLER_AXIS_LEFTX : SDL_CONTROLLER_AXIS_RIGHTX;
    const auto verticalAxis = leftStick ? SDL_CONTROLLER_AXIS_LEFTY : SDL_CONTROLLER_AXIS_RIGHTY;
    const int horizontal = SDL_GameControllerGetAxis(controller, horizontalAxis);
    const int vertical = SDL_GameControllerGetAxis(controller, verticalAxis);
    const int horizontalMagnitude = std::abs(horizontal);
    const int verticalMagnitude = std::abs(vertical);
    const int magnitude = std::max(horizontalMagnitude, verticalMagnitude);
    auto& state = stickStates_[stickIndex];

    // A separate release threshold prevents a stick resting near an edge from
    // repeatedly firing while the player returns it to the centre.
    if (magnitude < kReleaseThreshold) {
        state.activeAction = Action::None;
        state.nextRepeatAt = 0;
        return Action::None;
    }
    if (magnitude < kEngageThreshold) return Action::None;

    // Use only the dominant axis to avoid accidental diagonal navigation.
    Action action = Action::None;
    if (horizontalMagnitude >= verticalMagnitude)
        action = horizontal < 0 ? Action::Left : Action::Right;
    else
        action = vertical < 0 ? Action::Up : Action::Down;

    const Uint64 now = SDL_GetTicks();
    if (action != state.activeAction) {
        state.activeAction = action;
        state.nextRepeatAt = now + kInitialRepeatDelayMs;
        return action;
    }
    if (now < state.nextRepeatAt) return Action::None;

    state.nextRepeatAt = now + kRepeatIntervalMs;
    return action;
}
