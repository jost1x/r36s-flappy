#pragma once

#include <raylib.h>

#include <string>

#include "game_logic.h"
#include "parallax_background.h"
#include "sprite_manager.h"

class GameRenderer {
   public:
    GameRenderer(const SpriteManager& sprites, const ParallaxBackground& bg, const Font& font);

    void drawBackground(int screenWidth, int screenHeight) const;
    void drawGameWorld(const GameLogic& logic, int screenWidth, int screenHeight) const;
    void drawBird(const GameLogic& logic) const;
    void drawParticles(const GameLogic& logic) const;
    void drawHUD(const GameLogic& logic, int bestScore, bool muted) const;
    void drawFPS(bool showFPS, int screenWidth, int screenHeight) const;

   private:
    void drawPipes(const GameLogic& logic) const;

    const SpriteManager& sprites_;
    const ParallaxBackground& bg_;
    const Font& font_;
    static constexpr float kFontSpacing = 1.5F;

    void drawText(const std::string& text, int x, int y, int fontSize, Color color) const;
    void drawCenteredText(const std::string& text, int y, int fontSize, Color color, int screenWidth) const;
};
