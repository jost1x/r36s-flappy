#pragma once

#include <raylib.h>

#include <string>

#include "game_logic.h"
#include "settings.h"
#include "ui_action.h"

class UiRenderer {
   public:
    UiRenderer(const Font& font);

    UiAction drawReadyMenu(int bestScore) const;
    UiAction drawPauseMenu(bool muted) const;
    UiAction drawGameOverMenu(const GameLogic& logic, int bestScore) const;
    UiAction drawOptionsMenu(GameSettings& settings) const;

    const char* getMedal(const GameLogic& logic) const;
    Color getMedalColor(const GameLogic& logic) const;

   private:
    const Font& font_;
    static constexpr float kFontSpacing = 1.5F;

    void drawText(const std::string& text, int x, int y, int fontSize, Color color) const;
    void drawCenteredText(const std::string& text, int y, int fontSize, Color color) const;
};
