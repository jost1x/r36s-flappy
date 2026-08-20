#pragma once

#include <raylib.h>

#include <string>

#include "game_logic.h"
#include "settings.h"

class UiRenderer {
   public:
    UiRenderer(const Font& font);

    void drawReadyMenu(int bestScore) const;
    void drawPauseMenu(bool muted) const;
    void drawGameOverMenu(const GameLogic& logic, int bestScore) const;
    void drawOptionsMenu(GameSettings& settings, int& selection) const;

    const char* getMedal(const GameLogic& logic) const;
    Color getMedalColor(const GameLogic& logic) const;

   private:
    const Font& font_;
    static constexpr float kFontSpacing = 1.5F;

    void drawText(const std::string& text, int x, int y, int fontSize, Color color) const;
    void drawCenteredText(const std::string& text, int y, int fontSize, Color color) const;
};
