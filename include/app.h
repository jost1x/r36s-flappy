#pragma once

#include <memory>
#include <SDL2/SDL.h>

class WeatherClient;
class WeatherScreen;
class InputManager;

class App {
public:
    App();
    ~App();
    
    void run();
    
private:
    bool initialize();
    void handleEvents();
    void update();
    void render();
    void cleanup();
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* frameTexture = nullptr;
    void* display = nullptr;
    void* keypad = nullptr;
    std::unique_ptr<WeatherClient> weatherClient;
    std::unique_ptr<WeatherScreen> weatherScreen;
    std::unique_ptr<InputManager> inputManager;
    bool running = true;
    bool startPressed = false;
    bool backPressed = false;
    
    static constexpr int WINDOW_WIDTH = 640;
    static constexpr int WINDOW_HEIGHT = 480;
    static constexpr const char* WINDOW_TITLE = "R36S - Weather App";
};
