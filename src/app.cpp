#include "app.h"

#ifdef EMBEDDED_ASSETS
#include "embedded_assets.h"
#endif

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
constexpr Color kInk{31, 61, 86, 255};
constexpr Color kSkyTop{77, 185, 242, 255};
}  // namespace

void App::run() {
    settings_ = GameSettings::load();

    unsigned int windowFlags = FLAG_VSYNC_HINT;
    if (std::getenv("R36S_FULLSCREEN") != nullptr || settings_.fullscreen) {
        windowFlags |= FLAG_FULLSCREEN_MODE;
    }
    SetConfigFlags(windowFlags);
    InitWindow(Config::kScreenWidth, Config::kScreenHeight, "R36S Flappy Bird");
    if (!IsWindowReady()) {
        std::cerr << "raylib could not initialize the display backend\n";
        return;
    }
    SetExitKey(KEY_NULL);
    SetTargetFPS(Config::kTargetFPS);
    InitAudioDevice();
    loadFont();
    spriteMgr_.load();
    bg_.setCloudTexture(spriteMgr_.getCloudSprite());

    input_.update();
    if (input_.hasGamepad()) {
        std::cerr << "Gamepad detected: " << GetGamepadName(input_.gamepadIndex()) << "\n";
    } else {
        std::cerr << "No gamepad detected, using keyboard/mouse only\n";
    }
    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(kInk));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(kInk));
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Color{255, 226, 80, 255}));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(Color{255, 242, 150, 255}));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(Color{246, 175, 49, 255}));
    loadBestScore();
    game_.resetRound();

    transition_.start(ScreenTransition::Type::FadeIn, 0.8F);
    bgm_.setVolume(settings_.bgmVolume);
    bgm_.start();

    GameRenderer gameRenderer{spriteMgr_, bg_, font_};
    UiRenderer uiRenderer{font_};

    while (!WindowShouldClose()) {
        input_.update();
        if (input_.isQuitPressed()) {
            break;
        }
        if (input_.isMutePressed()) {
            soundMgr_.toggleMute();
        }
        if (input_.isOptionsPressed() && game_.getState() == GameState::Ready) {
            game_.setState(GameState::Options);
        }
        transition_.update(GetFrameTime());

        UiAction uiAction = handleInput(uiRenderer);
        if (uiAction != UiAction::None) {
            handleUiAction(uiAction);
        }

        game_.update(GetFrameTime());
        bg_.update(GetFrameTime(), Config::kScreenWidth);

        if (game_.didScore()) {
            soundMgr_.playPoint();
        }

        BeginDrawing();
        ClearBackground(kSkyTop);
        gameRenderer.drawGameWorld(game_, Config::kScreenWidth, Config::kScreenHeight);
        gameRenderer.drawHUD(game_, bestScore_, soundMgr_.isMuted());

        switch (game_.getState()) {
            case GameState::Ready:
                uiRenderer.drawReadyMenu(bestScore_);
                break;
            case GameState::Paused:
                uiRenderer.drawPauseMenu(false);
                break;
            case GameState::GameOver:
                uiRenderer.drawGameOverMenu(game_, bestScore_);
                break;
            case GameState::Options:
                uiRenderer.drawOptionsMenu(settings_);
                break;
            case GameState::Playing:
                break;
        }

        transition_.drawOverlay();
        gameRenderer.drawFPS(settings_.showFPS, Config::kScreenWidth, Config::kScreenHeight);
        EndDrawing();
    }

    settings_.save();
    saveBestScore();
    spriteMgr_.unload();
    unloadFont();
    CloseAudioDevice();
    CloseWindow();
}

UiAction App::handleInput(const UiRenderer& uiRenderer) {
    if (game_.getState() == GameState::Options) {
        if (input_.isFlapPressed() || input_.isPausePressed()) {
            return UiAction::Back;
        }
        return uiRenderer.drawOptionsMenu(settings_);
    }

    if (game_.getState() == GameState::Ready) {
        if (input_.isFlapPressed()) {
            return UiAction::Play;
        }
        if (input_.isOptionsPressed()) {
            return UiAction::Options;
        }
        return uiRenderer.drawReadyMenu(bestScore_);
    }

    if (game_.getState() == GameState::GameOver) {
        if (input_.isFlapPressed()) {
            return UiAction::Retry;
        }
        if (input_.isOptionsPressed()) {
            return UiAction::Menu;
        }
        return uiRenderer.drawGameOverMenu(game_, bestScore_);
    }

    if (game_.getState() == GameState::Paused) {
        if (input_.isPausePressed()) {
            return UiAction::Continue;
        }
        if (input_.isFlapPressed()) {
            return UiAction::Restart;
        }
        return uiRenderer.drawPauseMenu(false);
    }

    // Playing state - check gamepad/keyboard input
    if (input_.isPausePressed()) {
        handlePause();
    }

    if (input_.isFlapPressed()) {
        handleFlap();
    }

    if (game_.checkCollisions()) {
        handleCollision();
    }

    return UiAction::None;
}

void App::handleUiAction(UiAction action) {
    switch (action) {
        case UiAction::Play:
            game_.resetRound();
            game_.setState(GameState::Playing);
            handleFlap();
            break;
        case UiAction::Options:
            game_.setState(GameState::Options);
            break;
        case UiAction::Continue:
            game_.setState(GameState::Playing);
            bgm_.start();
            break;
        case UiAction::Restart:
            game_.resetRound();
            game_.setState(GameState::Playing);
            handleFlap();
            break;
        case UiAction::Retry:
            game_.resetRound();
            game_.setState(GameState::Playing);
            handleFlap();
            break;
        case UiAction::Menu:
            game_.resetRound();
            game_.setState(GameState::Ready);
            break;
        case UiAction::ToggleMute:
            soundMgr_.toggleMute();
            break;
        case UiAction::Back:
            applySettings();
            game_.setState(GameState::Ready);
            break;
        case UiAction::None:
            break;
    }
}

void App::handleFlap() {
    game_.getBird().flap(Config::kFlapVelocity);
    soundMgr_.playFlap();
}

void App::handleCollision() {
    game_.setState(GameState::GameOver);
    game_.setCollisionFlash(0.3F);
    soundMgr_.playHit();
    triggerCollisionVibration();
    bgm_.stop();
}

void App::handlePause() {
    transition_.start(ScreenTransition::Type::FadeOut, 0.3F);
    if (game_.getState() == GameState::Paused) {
        game_.setState(GameState::Playing);
        bgm_.start();
    } else {
        game_.setState(GameState::Paused);
        bgm_.stop();
    }
}

void App::loadFont() {
#ifdef EMBEDDED_ASSETS
    font_ = LoadFontFromMemory(".ttf", montserrat_font_data, montserrat_font_size, 48, nullptr, 0);
    if (font_.texture.id != 0) return;
    std::cerr << "Warning: Could not load embedded font, trying filesystem\n";
#endif
    std::string fontPath = resolveFontPath();
    font_ = LoadFontEx(fontPath.c_str(), 48, nullptr, 0);
    if (font_.texture.id == 0) {
        std::cerr << "Warning: Could not load font from " << fontPath << ", using default\n";
    }
}

void App::unloadFont() {
    if (font_.texture.id != 0) {
        UnloadFont(font_);
    }
}

std::string App::resolveFontPath() const {
    const char* exeDir = std::getenv("R36S_APP_DIR");
    if (exeDir) {
        std::filesystem::path p = std::string(exeDir) + "/assets/Montserrat-Medium.ttf";
        if (std::filesystem::exists(p)) return p.string();
    }
    std::filesystem::path local = "assets/Montserrat-Medium.ttf";
    if (std::filesystem::exists(local)) return local.string();
    local = "../assets/Montserrat-Medium.ttf";
    if (std::filesystem::exists(local)) return local.string();
    return "assets/Montserrat-Medium.ttf";
}

uint32_t App::computeDailySeed() const {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    return static_cast<uint32_t>(local->tm_year + 1900) * 10000 + static_cast<uint32_t>(local->tm_mon + 1) * 100 +
           static_cast<uint32_t>(local->tm_mday);
}

void App::triggerCollisionVibration() const {
    if (settings_.vibrationEnabled && input_.hasGamepad()) {
        SetGamepadVibration(input_.gamepadIndex(), Config::kCollisionVibrationStrength,
                            Config::kCollisionVibrationStrength,
                            Config::kCollisionVibrationDuration);
    }
}

void App::applySettings() { bgm_.setVolume(settings_.bgmVolume); }

void App::loadBestScore() {
    const char* dataHome = std::getenv("XDG_DATA_HOME");
    const char* home = std::getenv("HOME");
    const std::filesystem::path base = dataHome ? dataHome : (home ? std::string(home) + "/.local/share" : ".");
    scorePath_ = (base / "r36s-flappy" / "high-score.txt").string();
    std::ifstream file(scorePath_);
    if (file) file >> bestScore_;
}

void App::saveBestScore() const {
    if (scorePath_.empty()) return;
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(scorePath_).parent_path(), error);
    std::ofstream file(scorePath_, std::ios::trunc);
    if (file) file << bestScore_ << '\n';
}
