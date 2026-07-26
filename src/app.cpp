#include "app.h"

#include "input_manager.h"
#include "weather/weather_client.h"
#include "weather/weather_screen.h"
#include <lvgl.h>

#include <iostream>
#include <vector>

namespace {
SDL_Renderer* gRenderer = nullptr;
SDL_Texture* gTexture = nullptr;
std::vector<uint8_t> gDrawBuffer;

void sdlFlush(lv_display_t* display, const lv_area_t* area, uint8_t* pixels) {
    const int width = area->x2 - area->x1 + 1;
    const int height = area->y2 - area->y1 + 1;
    SDL_Rect destination{area->x1, area->y1, width, height};
    SDL_UpdateTexture(gTexture, &destination, pixels, width * 4);
    if (lv_display_flush_is_last(display)) {
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, gTexture, nullptr, nullptr);
        SDL_RenderPresent(gRenderer);
    }
    lv_display_flush_ready(display);
}
}  // namespace

App::App() = default;

App::~App() { cleanup(); }

bool App::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (SDL_getenv("R36S_FULLSCREEN") != nullptr) windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, windowFlags);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    frameTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                     WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!frameTexture) {
        std::cerr << "Failed to create LVGL texture: " << SDL_GetError() << std::endl;
        return false;
    }

    lv_init();
    gRenderer = renderer;
    gTexture = frameTexture;
    gDrawBuffer.resize(static_cast<size_t>(WINDOW_WIDTH) * 48U * 4U);
    auto* lvDisplay = lv_display_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_ARGB8888);
    lv_display_set_buffers(lvDisplay, gDrawBuffer.data(), nullptr, gDrawBuffer.size(), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(lvDisplay, sdlFlush);
    display = lvDisplay;

    inputManager = std::make_unique<InputManager>();
    inputManager->open();
    weatherClient = std::make_unique<WeatherClient>();
    weatherScreen = std::make_unique<WeatherScreen>(*weatherClient);
    weatherScreen->show();
    std::cout << "LVGL weather app initialized" << std::endl;
    return true;
}

void App::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { running = false; continue; }
        if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
            const bool pressed = event.type == SDL_CONTROLLERBUTTONDOWN;
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) startPressed = pressed;
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) backPressed = pressed;
            if (startPressed && backPressed) { running = false; continue; }
        }
        if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
            const bool pressed = event.type == SDL_JOYBUTTONDOWN;
            if (event.jbutton.button == 13) startPressed = pressed;
            if (event.jbutton.button == 12) backPressed = pressed;
            if (startPressed && backPressed) { running = false; continue; }
        }
        const Action action = inputManager->handleEvent(&event);
        if (action != Action::None) weatherScreen->handleAction(static_cast<int>(action));
    }
}

void App::update() { weatherScreen->update(); }

void App::render() { lv_timer_handler(); }

void App::cleanup() {
    weatherScreen.reset();
    weatherClient.reset();
    inputManager.reset();
    if (display) {
        lv_display_delete(static_cast<lv_display_t*>(display));
        display = nullptr;
    }
    if (gTexture) { gTexture = nullptr; }
    gRenderer = nullptr;
    gDrawBuffer.clear();
    lv_deinit();
    if (frameTexture) { SDL_DestroyTexture(frameTexture); frameTexture = nullptr; }
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    SDL_Quit();
}

void App::run() {
    if (!initialize()) return;
    Uint32 previousTick = SDL_GetTicks();
    while (running) {
        const Uint32 frameStart = SDL_GetTicks();
        lv_tick_inc(frameStart - previousTick);
        previousTick = frameStart;
        handleEvents();
        update();
        render();
        const Uint32 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < 16) SDL_Delay(16 - elapsed);
    }
}
