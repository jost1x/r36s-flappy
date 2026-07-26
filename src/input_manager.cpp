#include "../include/input_manager.h"
#include <iostream>

InputManager::InputManager() {}

InputManager::~InputManager() {
    close();
}

bool InputManager::open() {
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Failed to init game controller: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Try to open first connected controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Game controller opened: " << SDL_GameControllerName(controller) << std::endl;
                initialized = true;
                return true;
            }
        }
    }
    
    std::cout << "No game controller found, using keyboard" << std::endl;
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

bool InputManager::shouldExit(SDL_Event* event) const {
    if (event->type == SDL_QUIT) return true;
    
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        return true;
    }
    
    // SELECT + START on R36S (mapped as BACK + START)
    if (event->type == SDL_CONTROLLERBUTTONDOWN) {
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_BACK || 
            event->cbutton.button == SDL_CONTROLLER_BUTTON_START) {
            return true;
        }
    }
    
    return false;
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
        case SDL_CONTROLLER_BUTTON_A:
        case SDL_CONTROLLER_BUTTON_START:
            return Action::Confirm;
        case SDL_CONTROLLER_BUTTON_B:
        case SDL_CONTROLLER_BUTTON_BACK:
            return Action::Back;
        default:
            return Action::None;
    }
}

Action InputManager::handleControllerAxisEvent(const SDL_ControllerAxisEvent& axis) {
    const Sint16 AXIS_THRESHOLD = 20000;
    
    if (axis.value < -AXIS_THRESHOLD) {
        if (axis.axis == SDL_CONTROLLER_AXIS_LEFTX || axis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
            return Action::Left;
        } else if (axis.axis == SDL_CONTROLLER_AXIS_LEFTY || axis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
            return Action::Up;
        }
    } else if (axis.value > AXIS_THRESHOLD) {
        if (axis.axis == SDL_CONTROLLER_AXIS_LEFTX || axis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
            return Action::Right;
        } else if (axis.axis == SDL_CONTROLLER_AXIS_LEFTY || axis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
            return Action::Down;
        }
    }
    
    return Action::None;
}

void InputManager::setActionCallback(Action action, std::function<void()> callback) {
    // This would be implemented in a more complete version
    // using a map of callbacks for each action
}
