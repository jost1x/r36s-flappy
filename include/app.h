#pragma once

#include <raylib.h>

#include <cstdint>
#include <string>

#include "bgm_manager.h"
#include "game_logic.h"
#include "game_renderer.h"
#include "input_handler.h"
#include "parallax_background.h"
#include "screen_transition.h"
#include "settings.h"
#include "sprite_manager.h"
#include "ui_renderer.h"

class App {
   public:
    void run();

   private:
    void runGameLoop();
    void handleInput();
    void handleFlap();
    void handleCollision();
    void handlePause();
    void handleOptionsMenu();

    void loadBestScore();
    void saveBestScore() const;
    void loadFont();
    void unloadFont();
    std::string resolveFontPath() const;
    uint32_t computeDailySeed() const;
    void triggerCollisionVibration() const;
    void applySettings();

    GameLogic game_;
    InputHandler input_;
    SpriteManager spriteMgr_;
    ParallaxBackground bg_;
    BgmManager bgm_;
    ScreenTransition transition_;
    GameSettings settings_;
    Font font_{};

    int bestScore_ = 0;
    std::string scorePath_;
    int optionsSelection_ = 0;
};
