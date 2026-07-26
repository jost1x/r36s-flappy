#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <functional>

enum class Action {
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Back,
    Exit,
    None
};

class InputManager {
public:
    InputManager();
    ~InputManager();

    bool open();
    void close();

    Action handleEvent(SDL_Event* event);
    bool shouldExit(SDL_Event* event) const;

    void setActionCallback(Action action, std::function<void()> callback);

private:
    SDL_GameController* controller = nullptr;
    bool initialized = false;

    Action handleKeyboardEvent(const SDL_KeyboardEvent& key);
    Action handleControllerButtonEvent(const SDL_ControllerButtonEvent& btn, bool pressed);
    Action handleControllerAxisEvent(const SDL_ControllerAxisEvent& axis);
};
