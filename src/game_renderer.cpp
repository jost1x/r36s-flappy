#include "game_renderer.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr Color kSkyTop{77, 185, 242, 255};
constexpr Color kSkyBottom{178, 229, 247, 255};
constexpr Color kInk{31, 61, 86, 255};
constexpr Color kPipeGreen{73, 181, 67, 255};
constexpr Color kPipeLight{129, 216, 91, 255};
constexpr Color kPipeDark{39, 132, 52, 255};

void drawTiledPipeBody(Texture2D texture, float x, float y, float height) {
    float drawY = y;
    float remaining = height;
    while (remaining > 0.0F) {
        float sliceHeight = std::min(remaining, static_cast<float>(texture.height));
        Rectangle source{0, 0, static_cast<float>(texture.width), sliceHeight};
        DrawTextureRec(texture, source, Vector2{x, drawY}, WHITE);
        drawY += sliceHeight;
        remaining -= sliceHeight;
    }
}
}  // namespace

GameRenderer::GameRenderer(const SpriteManager& sprites, const ParallaxBackground& bg, const Font& font)
    : sprites_(sprites), bg_(bg), font_(font) {}

void GameRenderer::drawText(const std::string& text, int x, int y, int fontSize, Color color) const {
    if (font_.texture.id != 0) {
        DrawTextEx(font_, text.c_str(), Vector2{static_cast<float>(x), static_cast<float>(y)},
                   static_cast<float>(fontSize), kFontSpacing, color);
    } else {
        DrawText(text.c_str(), x, y, fontSize, color);
    }
}

void GameRenderer::drawCenteredText(const std::string& text, int y, int fontSize, Color color, int screenWidth) const {
    float textWidth = 0.0F;
    if (font_.texture.id != 0) {
        textWidth = MeasureTextEx(font_, text.c_str(), static_cast<float>(fontSize), kFontSpacing).x;
    } else {
        textWidth = static_cast<float>(MeasureText(text.c_str(), fontSize));
    }
    int x = static_cast<int>((static_cast<float>(screenWidth) - textWidth) / 2.0F);
    drawText(text, x, y, fontSize, color);
}

void GameRenderer::drawBackground(int screenWidth, int screenHeight) const {
    DrawRectangleGradientV(0, 0, screenWidth, static_cast<int>(Config::kGroundY), kSkyTop, kSkyBottom);
    bg_.draw(screenWidth, static_cast<int>(Config::kGroundY));
    (void)screenHeight;
}

void GameRenderer::drawPipes(const GameLogic& logic) const {
    if (sprites_.isLoaded()) {
        const auto& pipes = logic.getPipeManager().getPipes();
        float currentGap = logic.getPipeManager().getCurrentGap();
        Texture2D topBodyTex = sprites_.getTopPipeBodySprite();
        Texture2D topLipTex = sprites_.getTopPipeLipSprite();
        Texture2D bottomBodyTex = sprites_.getBottomPipeBodySprite();
        Texture2D bottomLipTex = sprites_.getBottomPipeLipSprite();
        for (const auto& pipe : pipes) {
            const float pipeX = std::round(pipe.x);
            const float gapTop = std::round(pipe.gapCenter - currentGap / 2.0F);
            const float gapBottom = std::round(pipe.gapCenter + currentGap / 2.0F);
            const float lipHeight = static_cast<float>(topLipTex.height);

            drawTiledPipeBody(topBodyTex, pipeX, 0.0F, gapTop - lipHeight);

            drawTiledPipeBody(bottomBodyTex, pipeX, gapBottom + lipHeight, Config::kGroundY - gapBottom - lipHeight);

            DrawTexture(topLipTex, static_cast<int>(pipeX), static_cast<int>(gapTop - lipHeight), WHITE);
            DrawTexture(bottomLipTex, static_cast<int>(pipeX), static_cast<int>(gapBottom), WHITE);
        }
    } else {
        logic.getPipeManager().drawAll(Config::kPipeWidth, Config::kPipeGap, Config::kGroundY, kPipeGreen, kPipeLight,
                                       kPipeDark);
    }
}

void GameRenderer::drawGameWorld(const GameLogic& logic, int screenWidth, int screenHeight) const {
    drawBackground(screenWidth, screenHeight);
    drawPipes(logic);
    bg_.drawGround(screenWidth, screenHeight, static_cast<int>(Config::kGroundY));
    drawBird(logic);
    drawParticles(logic);

    if (logic.getCollisionFlash() > 0) {
        DrawRectangle(0, 0, screenWidth, screenHeight,
                      Color{255, 255, 255, static_cast<unsigned char>(logic.getCollisionFlash() / 0.3F * 180)});
    }
}

void GameRenderer::drawBird(const GameLogic& logic) const {
    const auto& bird = logic.getBird();
    if (sprites_.isLoaded()) {
        constexpr double wingFrameSeconds = 0.11;
        const bool wingsUp = static_cast<int>(GetTime() / wingFrameSeconds) % 2 != 0;
        Texture2D birdTex = sprites_.getBirdSprite(wingsUp);
        Rectangle source{0, 0, static_cast<float>(birdTex.width), static_cast<float>(birdTex.height)};
        const float birdWidth = static_cast<float>(birdTex.width);
        const float birdHeight = static_cast<float>(birdTex.height);
        Rectangle destination{Config::kBirdX - birdWidth / 2.0F, bird.getY() - birdHeight / 2.0F, birdWidth,
                              birdHeight};
        DrawTexturePro(birdTex, source, destination, Vector2{birdWidth / 2.0F, birdHeight / 2.0F}, bird.getRotation(),
                       WHITE);
    } else {
        bird.draw();
    }
}

void GameRenderer::drawParticles(const GameLogic& logic) const { logic.getParticles().draw(); }

void GameRenderer::drawHUD(const GameLogic& logic, int bestScore, bool muted) const {
    drawCenteredText(TextFormat("%02i", logic.getPipeManager().getScore()), 30, 48, RAYWHITE, Config::kScreenWidth);
    drawText(TextFormat("RECORD %02i", bestScore), 16, 16, 18, Fade(kInk, 0.82F));

    int level = logic.getPipeManager().getLevel();
    if (level > 0) {
        std::string levelText = TextFormat("NIVEL %i", level + 1);
        float textWidth = font_.texture.id != 0 ? MeasureTextEx(font_, levelText.c_str(), 14.0F, kFontSpacing).x
                                                : static_cast<float>(MeasureText(levelText.c_str(), 14));
        drawText(levelText, static_cast<int>(Config::kScreenWidth - textWidth - 16), 16, 14, Fade(kInk, 0.7F));
    }

    if (logic.getScorePopupTimer() > 0) {
        float alpha = logic.getScorePopupTimer() / 0.5F;
        float y = logic.getScorePopupY() - (1.0F - alpha) * 30.0F;
        Color popupColor = Color{255, 255, 255, static_cast<unsigned char>(alpha * 255)};
        drawCenteredText("+1", static_cast<int>(y), 24, popupColor, Config::kScreenWidth);
    }

    if (muted) {
        drawText("MUTE", Config::kScreenWidth - 50, 16, 14, Fade(kInk, 0.6F));
    }
}

void GameRenderer::drawFPS(bool showFPS, int screenWidth, int screenHeight) const {
    if (showFPS) {
        int fps = GetFPS();
        drawText(TextFormat("%i FPS", fps), screenWidth - 80, screenHeight - 24, 12, Fade(kInk, 0.5F));
    }
}
