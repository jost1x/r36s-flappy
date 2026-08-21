#include "ui_renderer.h"

#include <algorithm>

namespace {
constexpr Color kInk{31, 61, 86, 255};

void drawPanel(Rectangle bounds, const char* title) {
    DrawRectangleRec(bounds, Color{235, 248, 252, 245});
    DrawRectangleLinesEx(bounds, 3.0F, kInk);
    int titleWidth = MeasureText(title, 20);
    DrawRectangle(static_cast<int>(bounds.x + 16.0F), static_cast<int>(bounds.y - 12.0F), titleWidth + 16, 28,
                  Color{235, 248, 252, 255});
    DrawText(title, static_cast<int>(bounds.x + 24.0F), static_cast<int>(bounds.y - 8.0F), 20, kInk);
}

struct ReadyPanel {
    static constexpr float x = 142.0F;
    static constexpr float y = 88.0F;
    static constexpr float w = 356.0F;
    static constexpr float h = 260.0F;
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

void UiRenderer::drawMenuItem(const char* text, Rectangle bounds, bool selected) const {
    Color fill = selected ? Color{255, 226, 80, 255} : Color{219, 239, 247, 255};
    Color border = selected ? Color{246, 175, 49, 255} : kInk;
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, selected ? 3.0F : 1.0F, border);
    int textWidth = font_.texture.id != 0 ? static_cast<int>(MeasureTextEx(font_, text, 18.0F, kFontSpacing).x)
                                          : MeasureText(text, 18);
    float centeredX = bounds.x + (bounds.width - static_cast<float>(textWidth)) / 2.0F;
    drawText(text, static_cast<int>(centeredX), static_cast<int>(bounds.y + 8.0F), 18, kInk);
}

void UiRenderer::drawReadyMenu(int bestScore, int selection) const {
    Rectangle panel{ReadyPanel::x, ReadyPanel::y, ReadyPanel::w, ReadyPanel::h};
    drawPanel(panel, "FLAPPY BIRD");
    drawCenteredText("CRUZA LAS TUBERIAS Y SUPERA TU RECORD", 132, 15, kInk);
    drawText(TextFormat("RECORD: %02i", bestScore), 260, 165, 20, kInk);
    drawMenuItem("JUGAR", {222.0F, 180.0F, 196.0F, 40.0F}, selection == 0);
    drawMenuItem("OPCIONES", {222.0F, 230.0F, 196.0F, 40.0F}, selection == 1);
    drawCenteredText("CRUCETA: MOVER", 290, 14, kInk);
    drawCenteredText("A: ELEGIR     SELECT: OPCIONES", 310, 14, kInk);
}

void UiRenderer::drawPauseMenu(bool muted, int selection) const {
    DrawRectangle(0, 0, Config::kScreenWidth, Config::kScreenHeight, Fade(BLACK, 0.35F));
    Rectangle panel{PausePanel::x, PausePanel::y, PausePanel::w, PausePanel::h};
    drawPanel(panel, "PAUSA");
    drawMenuItem("CONTINUAR", {222.0F, 184.0F, 196.0F, 36.0F}, selection == 0);
    drawMenuItem("REINICIAR", {222.0F, 228.0F, 196.0F, 36.0F}, selection == 1);
    std::string muteText = muted ? "ACTIVAR SONIDO" : "SILENCIAR";
    drawMenuItem(muteText.c_str(), {222.0F, 272.0F, 196.0F, 36.0F}, selection == 2);
    drawCenteredText("A: ELEGIR   B: CONTINUAR", 326, 14, kInk);
}

void UiRenderer::drawGameOverMenu(const GameLogic& logic, int bestScore, int selection) const {
    Rectangle panel{GameOverPanel::x, GameOverPanel::y, GameOverPanel::w, GameOverPanel::h};
    drawPanel(panel, "FIN DEL VUELO");
    drawCenteredText(TextFormat("PUNTOS: %i     RECORD: %i", logic.getPipeManager().getScore(), bestScore), 165, 18,
                     kInk);

    const char* medal = getMedal(logic);
    if (medal) {
        Color medalColor = getMedalColor(logic);
        drawCenteredText(medal, 200, 28, medalColor);
    }

    drawMenuItem("REINTENTAR", {222.0F, 234.0F, 196.0F, 38.0F}, selection == 0);
    drawMenuItem("MENU", {222.0F, 282.0F, 196.0F, 38.0F}, selection == 1);
    drawCenteredText("A: ELEGIR   B: MENU", 338, 14, kInk);
}

void UiRenderer::drawOptionsMenu(const GameSettings& settings, int selection) const {
    DrawRectangle(0, 0, Config::kScreenWidth, Config::kScreenHeight, Fade(BLACK, 0.5F));
    Rectangle panel{142.0F, 60.0F, 356.0F, 340.0F};
    drawPanel(panel, "OPCIONES");

    float y = 100.0F;

    drawMenuItem(TextFormat("SFX: %i%%", static_cast<int>(settings.sfxVolume * 100.0F)), {222.0F, y, 196.0F, 30.0F},
                 selection == 0);
    y += 40.0F;

    drawMenuItem(TextFormat("MUSICA: %i%%", static_cast<int>(settings.bgmVolume * 100.0F)), {222.0F, y, 196.0F, 30.0F},
                 selection == 1);
    y += 40.0F;

    drawMenuItem(settings.vibrationEnabled ? "VIBRACION: SI" : "VIBRACION: NO", {222.0F, y, 196.0F, 30.0F},
                 selection == 2);
    y += 36.0F;

    drawMenuItem(settings.showFPS ? "MOSTRAR FPS: SI" : "MOSTRAR FPS: NO", {222.0F, y, 196.0F, 30.0F}, selection == 3);
    y += 40.0F;

    drawMenuItem("VOLVER", {222.0F, y, 196.0F, 34.0F}, selection == 4);
    drawCenteredText("IZQ./DER.: AJUSTAR   A: CAMBIAR   B: VOLVER", 368, 13, kInk);
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
