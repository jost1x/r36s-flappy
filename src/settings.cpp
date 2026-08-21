#include "settings.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
bool parseBoolSetting(const std::string& value, bool& output) {
    if (value == "1") {
        output = true;
        return true;
    }
    if (value == "0") {
        output = false;
        return true;
    }
    return false;
}
}  // namespace

std::string GameSettings::settingsPath() { return (std::filesystem::path(dataDirectory()) / "settings.ini").string(); }

std::string GameSettings::dataDirectory() {
    const char* r36sDataDir = std::getenv("R36S_DATA_DIR");
    if (r36sDataDir != nullptr && r36sDataDir[0] != '\0') return r36sDataDir;

    const char* appDir = std::getenv("R36S_APP_DIR");
    if (appDir != nullptr && appDir[0] != '\0') {
        return (std::filesystem::path(appDir) / "data").string();
    }

    const char* dataHome = std::getenv("XDG_DATA_HOME");
    const char* home = std::getenv("HOME");
    const std::filesystem::path base = dataHome ? dataHome : (home ? std::string(home) + "/.local/share" : ".");
    return (base / "r36s-flappy").string();
}

GameSettings GameSettings::load() {
    GameSettings settings;
    std::string path = settingsPath();
    return loadFromPath(path);
}

GameSettings GameSettings::loadFromPath(const std::string& path) {
    GameSettings settings;
    std::ifstream file(path);
    if (!file) return settings;

    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key == "sfx_volume" || key == "bgm_volume") {
            try {
                float parsed = std::stof(value);
                if (key == "sfx_volume")
                    settings.sfxVolume = parsed;
                else
                    settings.bgmVolume = parsed;
            } catch (const std::exception&) {
                std::cerr << "Warning: Ignoring invalid " << key << " in " << path << "\n";
            }
        } else if (key == "vibration" || key == "fullscreen" || key == "show_fps") {
            bool* target = key == "vibration"    ? &settings.vibrationEnabled
                           : key == "fullscreen" ? &settings.fullscreen
                                                 : &settings.showFPS;
            if (!parseBoolSetting(value, *target)) {
                std::cerr << "Warning: Ignoring invalid " << key << " in " << path << "\n";
            }
        }
    }

    settings.sfxVolume = std::clamp(settings.sfxVolume, 0.0F, 1.0F);
    settings.bgmVolume = std::clamp(settings.bgmVolume, 0.0F, 1.0F);
    return settings;
}

void GameSettings::save() const {
    std::string path = settingsPath();
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), error);
    if (error) {
        std::cerr << "Warning: Could not create settings directory for " << path << ": " << error.message() << "\n";
        return;
    }

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
