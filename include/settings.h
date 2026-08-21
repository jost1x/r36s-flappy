#pragma once

#include <string>

struct GameSettings {
    float sfxVolume = 0.3F;
    float bgmVolume = 0.15F;
    bool vibrationEnabled = true;
    bool fullscreen = false;
    bool showFPS = false;

    static GameSettings load();
    static GameSettings loadFromPath(const std::string& path);
    static std::string dataDirectory();
    void save() const;

   private:
    static std::string settingsPath();
};
