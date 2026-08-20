#include "ui_renderer.h"

#include <raygui.h>

#include <algorithm>

namespace {
constexpr Color kInk{31, 61, 86, 255};

struct ReadyPanel {
    static constexpr float x = 142.0F;
    static constexpr float y = 98.0F;
    static constexpr float w = 356.0F;
    static constexpr float h = 240.0F;
};

struct PausePanel {
    static constexpr float x = 176.0F;
    static constexpr float y = 144.0F;
    static constexpr float w = 288.0F;
    static constexpr float h = 210.0F;
};

struct GameOverPanel {
    static constexpr float x = 142.0F;
    static constexpr float y = 118.0F;
    static constexpr float w = 356.0F;
    static constexpr float h = 250.0F;
};
}  // namespace

UiRenderer::UiRenderer(const Font& font) : font_(font) {}

void UiRenderer::drawText(const std::string& text, int x, int y, int fontSize, Color color) const {
    if (font_.texture.id != 0) {
        DrawTextEx(font_, text.c_str(), Vector2{static_cast<float>(x), static_cast<float>(y)},
                   static_cast<float>(fontSize), kFontSpacing, color);
    } else {
        DrawText(text.c_str(), x, y, fontSize, color);
    }
}

void UiRenderer::drawCenteredText(const std::string& text, int y, int fontSize, Color color) const {
    float textWidth = 0.0F;
    if (font_.texture.id != 0) {
        textWidth = MeasureTextEx(font_, text.c_str(), static_cast<float>(fontSize), kFontSpacing).x;
    } else {
        textWidth = static_cast<float>(MeasureText(text.c_str(), fontSize));
    }
    int x = static_cast<int>((static_cast<float>(Config::kScreenWidth) - textWidth) / 2.0F);
    drawText(text, x, y, fontSize, color);
}

UiAction UiRenderer::drawReadyMenu(int bestScore) const {
    Rectangle panel{ReadyPanel::x, ReadyPanel::y, ReadyPanel::w, ReadyPanel::h};
    GuiPanel(panel, "FLAPPY BIRD");
    GuiLabel({176.0F, 142.0F, 288.0F, 24.0F}, "CRUZA LAS TUBERIAS Y SUPERA TU RECORD");
    drawText(TextFormat("RECORD: %02i", bestScore), 260, 175, 20, kInk);
    if (GuiButton({222.0F, 190.0F, 196.0F, 44.0F}, "JUGAR")) {
        return UiAction::Play;
    }
    GuiLabel({180.0F, 258.0F, 280.0F, 22.0F}, "A/B, ESPACIO O CLIC PARA VOLAR");
    if (GuiButton({222.0F, 290.0F, 196.0F, 32.0F}, "OPCIONES")) {
        return UiAction::Options;
    }
    return UiAction::None;
}

UiAction UiRenderer::drawPauseMenu(bool muted) const {
    DrawRectangle(0, 0, Config::kScreenWidth, Config::kScreenHeight, Fade(BLACK, 0.35F));
    Rectangle panel{PausePanel::x, PausePanel::y, PausePanel::w, PausePanel::h};
    GuiPanel(panel, "PAUSA");
    if (GuiButton({222.0F, 194.0F, 196.0F, 40.0F}, "CONTINUAR")) {
        return UiAction::Continue;
    }
    if (GuiButton({222.0F, 248.0F, 196.0F, 40.0F}, "REINICIAR")) {
        return UiAction::Restart;
    }
    std::string muteText = muted ? "ACTIVAR SONIDO" : "SILENCIAR";
    if (GuiButton({222.0F, 300.0F, 196.0F, 36.0F}, muteText.c_str())) {
        return UiAction::ToggleMute;
    }
    return UiAction::None;
}

UiAction UiRenderer::drawGameOverMenu(const GameLogic& logic, int bestScore) const {
    Rectangle panel{GameOverPanel::x, GameOverPanel::y, GameOverPanel::w, GameOverPanel::h};
    GuiPanel(panel, "FIN DEL VUELO");
    GuiLabel({190.0F, 165.0F, 260.0F, 30.0F},
             TextFormat("PUNTOS: %i     RECORD: %i", logic.getPipeManager().getScore(), bestScore));

    const char* medal = getMedal(logic);
    if (medal) {
        Color medalColor = getMedalColor(logic);
        drawCenteredText(medal, 200, 28, medalColor);
    }

    if (GuiButton({222.0F, 234.0F, 196.0F, 42.0F}, "REINTENTAR")) {
        return UiAction::Retry;
    }
    if (GuiButton({222.0F, 290.0F, 196.0F, 36.0F}, "MENU")) {
        return UiAction::Menu;
    }
    return UiAction::None;
}

UiAction UiRenderer::drawOptionsMenu(GameSettings& settings) const {
    DrawRectangle(0, 0, Config::kScreenWidth, Config::kScreenHeight, Fade(BLACK, 0.5F));
    Rectangle panel{142.0F, 60.0F, 356.0F, 340.0F};
    GuiPanel(panel, "OPCIONES");

    float y = 100.0F;
    float labelWidth = 160.0F;
    float sliderWidth = 160.0F;
    float sliderX = 222.0F + labelWidth;

    GuiLabel({222.0F, y, labelWidth, 24.0F}, "Volumen SFX");
    GuiSlider({sliderX, y + 4, sliderWidth, 16}, nullptr, nullptr, &settings.sfxVolume, 0.0F, 1.0F);
    y += 40.0F;

    GuiLabel({222.0F, y, labelWidth, 24.0F}, "Volumen Musica");
    GuiSlider({sliderX, y + 4, sliderWidth, 16}, nullptr, nullptr, &settings.bgmVolume, 0.0F, 1.0F);
    y += 40.0F;

    GuiCheckBox({222.0F, y, 20.0F, 20.0F}, "Vibracion", &settings.vibrationEnabled);
    y += 36.0F;

    GuiCheckBox({222.0F, y, 20.0F, 20.0F}, "Mostrar FPS", &settings.showFPS);
    y += 40.0F;

    if (GuiButton({222.0F, y, 196.0F, 36.0F}, "VOLVER")) {
        return UiAction::Back;
    }
    return UiAction::None;
}

const char* UiRenderer::getMedal(const GameLogic& logic) const {
    int score = logic.getPipeManager().getScore();
    if (score >= Config::kMedalPlatinum) return "MEDALLA DE PLATINO";
    if (score >= Config::kMedalDiamond) return "MEDALLA DE DIAMANTE";
    if (score >= Config::kMedalGold) return "MEDALLA DE ORO";
    if (score >= Config::kMedalSilver) return "MEDALLA DE PLATA";
    if (score >= Config::kMedalBronze) return "MEDALLA DE BRONCE";
    return nullptr;
}

Color UiRenderer::getMedalColor(const GameLogic& logic) const {
    int score = logic.getPipeManager().getScore();
    if (score >= Config::kMedalPlatinum) return Color{229, 229, 229, 255};
    if (score >= Config::kMedalDiamond) return Color{185, 242, 255, 255};
    if (score >= Config::kMedalGold) return Color{255, 215, 0, 255};
    if (score >= Config::kMedalSilver) return Color{192, 192, 192, 255};
    if (score >= Config::kMedalBronze) return Color{205, 127, 50, 255};
    return WHITE;
}
