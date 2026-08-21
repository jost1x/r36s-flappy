#include "app.h"

#ifdef EMBEDDED_ASSETS
#include "embedded_assets.h"
#endif

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
    loadBestScore();
    game_.resetRound();

    transition_.start(ScreenTransition::Type::FadeIn, 0.8F);
    soundMgr_.initialize();
    soundMgr_.setVolume(settings_.sfxVolume);
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
            bgm_.setMuted(soundMgr_.isMuted());
        }
        transition_.update(GetFrameTime());
        bgm_.update();

        UiAction uiAction = handleInput();
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
                uiRenderer.drawReadyMenu(bestScore_, menuSelection_);
                break;
            case GameState::Paused:
                uiRenderer.drawPauseMenu(soundMgr_.isMuted(), menuSelection_);
                break;
            case GameState::GameOver:
                uiRenderer.drawGameOverMenu(game_, bestScore_, menuSelection_);
                break;
            case GameState::Options:
                uiRenderer.drawOptionsMenu(settings_, menuSelection_);
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
    bgm_.shutdown();
    soundMgr_.shutdown();
    CloseAudioDevice();
    CloseWindow();
}

UiAction App::handleInput() {
    if (game_.getState() != GameState::Playing) return handleMenuInput();

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

void App::moveMenuSelection(int itemCount, int direction) {
    menuSelection_ = (menuSelection_ + direction + itemCount) % itemCount;
}

void App::adjustSelectedOption(float direction) {
    if (menuSelection_ == 0) {
        settings_.sfxVolume = std::clamp(settings_.sfxVolume + direction * 0.05F, 0.0F, 1.0F);
        soundMgr_.setVolume(settings_.sfxVolume);
    } else if (menuSelection_ == 1) {
        settings_.bgmVolume = std::clamp(settings_.bgmVolume + direction * 0.05F, 0.0F, 1.0F);
        bgm_.setVolume(settings_.bgmVolume);
    } else if (menuSelection_ == 2) {
        settings_.vibrationEnabled = !settings_.vibrationEnabled;
    } else if (menuSelection_ == 3) {
        settings_.showFPS = !settings_.showFPS;
    }
}

UiAction App::handleMenuInput() {
    const GameState state = game_.getState();
    const int itemCount = state == GameState::Options ? 5 : (state == GameState::Paused ? 3 : 2);
    if (input_.isMenuUpPressed()) moveMenuSelection(itemCount, -1);
    if (input_.isMenuDownPressed()) moveMenuSelection(itemCount, 1);

    if (state == GameState::Options) {
        if (input_.isMenuLeftPressed()) adjustSelectedOption(-1.0F);
        if (input_.isMenuRightPressed()) adjustSelectedOption(1.0F);
        if (input_.isConfirmPressed()) {
            if (menuSelection_ == 4) return UiAction::Back;
            if (menuSelection_ >= 2) adjustSelectedOption(1.0F);
        }
        if (input_.isBackPressed() || input_.isPausePressed()) return UiAction::Back;
        return UiAction::None;
    }

    if (state == GameState::Ready && input_.isOptionsPressed()) return UiAction::Options;
    if (state == GameState::Paused && (input_.isBackPressed() || input_.isPausePressed())) return UiAction::Continue;
    if (state == GameState::GameOver && input_.isBackPressed()) return UiAction::Menu;
    if (!input_.isConfirmPressed()) return UiAction::None;

    if (state == GameState::Ready) return menuSelection_ == 0 ? UiAction::Play : UiAction::Options;
    if (state == GameState::Paused) {
        return menuSelection_ == 0 ? UiAction::Continue
                                   : (menuSelection_ == 1 ? UiAction::Restart : UiAction::ToggleMute);
    }
    return menuSelection_ == 0 ? UiAction::Retry : UiAction::Menu;
}

void App::handleUiAction(UiAction action) {
    switch (action) {
        case UiAction::Play:
            menuSelection_ = 0;
            game_.resetRound();
            game_.setState(GameState::Playing);
            handleFlap();
            break;
        case UiAction::Options:
            menuSelection_ = 0;
            game_.setState(GameState::Options);
            break;
        case UiAction::Continue:
            menuSelection_ = 0;
            game_.setState(GameState::Playing);
            bgm_.start();
            break;
        case UiAction::Restart:
            menuSelection_ = 0;
            game_.resetRound();
            game_.setState(GameState::Playing);
            handleFlap();
            break;
        case UiAction::Retry:
            menuSelection_ = 0;
            game_.resetRound();
            game_.setState(GameState::Playing);
            handleFlap();
            break;
        case UiAction::Menu:
            menuSelection_ = 0;
            game_.resetRound();
            game_.setState(GameState::Ready);
            break;
        case UiAction::ToggleMute:
            soundMgr_.toggleMute();
            bgm_.setMuted(soundMgr_.isMuted());
            break;
        case UiAction::Back:
            applySettings();
            menuSelection_ = 0;
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
    menuSelection_ = 0;
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
                            Config::kCollisionVibrationStrength, Config::kCollisionVibrationDuration);
    }
}

void App::applySettings() {
    soundMgr_.setVolume(settings_.sfxVolume);
    bgm_.setVolume(settings_.bgmVolume);
}

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
