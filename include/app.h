#pragma once

#include <SDL2/SDL.h>

#include <memory>

class WeatherClient;
class WeatherScreen;
class InputManager;
class SettingsStore;

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
    std::unique_ptr<SettingsStore> settingsStore;
    bool running = true;

    static constexpr int WINDOW_WIDTH = 640;
    static constexpr int WINDOW_HEIGHT = 480;
    static constexpr const char* WINDOW_TITLE = "R36S - Weather App";
};
