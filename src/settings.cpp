#include "settings.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
constexpr float kDefaultSfxVolume = 0.3F;
constexpr float kDefaultBgmVolume = 0.15F;
}  // namespace

std::string GameSettings::settingsPath() {
    const char* dataHome = std::getenv("XDG_DATA_HOME");
    const char* home = std::getenv("HOME");
    const std::filesystem::path base = dataHome ? dataHome : (home ? std::string(home) + "/.local/share" : ".");
    return (base / "r36s-flappy" / "settings.ini").string();
}

GameSettings GameSettings::load() {
    GameSettings settings;
    std::string path = settingsPath();
    std::ifstream file(path);
    if (!file) return settings;

    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key == "sfx_volume")
            settings.sfxVolume = std::stof(value);
        else if (key == "bgm_volume")
            settings.bgmVolume = std::stof(value);
        else if (key == "vibration")
            settings.vibrationEnabled = (value == "1");
        else if (key == "fullscreen")
            settings.fullscreen = (value == "1");
        else if (key == "show_fps")
            settings.showFPS = (value == "1");
    }

    settings.sfxVolume = std::clamp(settings.sfxVolume, 0.0F, 1.0F);
    settings.bgmVolume = std::clamp(settings.bgmVolume, 0.0F, 1.0F);
    return settings;
}

void GameSettings::save() const {
    std::string path = settingsPath();
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), error);

    std::ofstream file(path, std::ios::trunc);
    if (!file) {
        std::cerr << "Warning: Could not save settings to " << path << "\n";
        return;
    }

    file << "sfx_volume=" << sfxVolume << "\n";
    file << "bgm_volume=" << bgmVolume << "\n";
    file << "vibration=" << (vibrationEnabled ? 1 : 0) << "\n";
    file << "fullscreen=" << (fullscreen ? 1 : 0) << "\n";
    file << "show_fps=" << (showFPS ? 1 : 0) << "\n";
}
