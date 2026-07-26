#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "weather_client.h"

struct AppSettings {
    std::string activeCityId = "santiago";
    std::vector<std::string> favorites{"santiago", "valparaiso", "concepcion", "puerto-montt"};
};

class SettingsStore {
   public:
    explicit SettingsStore(std::string path = {});

    bool load();
    bool save() const;
    const AppSettings& settings() const { return settings_; }
    void setActiveCity(const std::string& id);
    bool addFavorite(const std::string& id);
    bool removeFavorite(const std::string& id);
    bool moveFavorite(const std::string& id, int direction);
    bool isFavorite(const std::string& id) const;

    void putCache(const WeatherData& data);
    std::optional<WeatherData> cached(const std::string& cityId, long long now, long long maxAgeSeconds) const;
    const std::string& path() const { return path_; }

   private:
    std::string path_;
    AppSettings settings_;
    std::unordered_map<std::string, WeatherData> cache_;
};
