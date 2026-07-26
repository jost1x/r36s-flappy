#include "weather/settings_store.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {
std::string defaultPath() {
    if (const char* configured = std::getenv("R36S_APP_DATA_DIR")) return std::string(configured) + "/weather.json";
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) return std::string(xdg) + "/r36s-weather/weather.json";
    if (const char* home = std::getenv("HOME")) return std::string(home) + "/.local/share/r36s-weather/weather.json";
    return "r36s-weather.json";
}
nlohmann::json serialize(const WeatherData& data) {
    nlohmann::json daily = nlohmann::json::array();
    for (const auto& day : data.daily)
        daily.push_back({{"date", day.date},
                         {"weatherCode", day.weatherCode},
                         {"maximum", day.maximum},
                         {"minimum", day.minimum},
                         {"precipitationProbability", day.precipitationProbability},
                         {"precipitation", day.precipitation}});
    return {{"weatherCode", data.weatherCode},
            {"temperature", data.temperature},
            {"apparentTemperature", data.apparentTemperature},
            {"humidity", data.humidity},
            {"windSpeed", data.windSpeed},
            {"precipitationProbability", data.precipitationProbability},
            {"precipitation", data.precipitation},
            {"updatedAt", data.updatedAt},
            {"daily", daily}};
}
std::optional<WeatherData> deserialize(const std::string& id, const nlohmann::json& json) {
    const auto* location = weatherLocationById(id);
    if (!location) return std::nullopt;
    try {
        WeatherData data{};
        data.location = *location;
        data.status = WeatherStatus::Ready;
        data.weatherCode = json.at("weatherCode").get<int>();
        data.temperature = json.at("temperature").get<int>();
        data.apparentTemperature = json.value("apparentTemperature", data.temperature);
        data.humidity = json.at("humidity").get<int>();
        data.windSpeed = json.at("windSpeed").get<int>();
        data.precipitationProbability = json.value("precipitationProbability", 0);
        data.precipitation = json.value("precipitation", 0.0);
        data.updatedAt = json.at("updatedAt").get<long long>();
        const auto& days = json.at("daily");
        if (days.size() < data.daily.size()) return std::nullopt;
        for (size_t index = 0; index < data.daily.size(); ++index) {
            const auto& day = days.at(index);
            data.daily[index] = {day.at("date").get<std::string>(),
                                 day.at("weatherCode").get<int>(),
                                 day.at("maximum").get<int>(),
                                 day.at("minimum").get<int>(),
                                 day.value("precipitationProbability", 0),
                                 day.value("precipitation", 0.0)};
        }
        return data;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}
}  // namespace

SettingsStore::SettingsStore(std::string path) : path_(path.empty() ? defaultPath() : std::move(path)) {}

bool SettingsStore::load() {
    std::ifstream input(path_);
    if (!input) return false;
    try {
        nlohmann::json root;
        input >> root;
        settings_.activeCityId = root.value("activeCityId", "santiago");
        settings_.favorites = root.value("favorites", std::vector<std::string>{"santiago"});
        settings_.favorites.erase(std::remove_if(settings_.favorites.begin(), settings_.favorites.end(),
                                                 [](const auto& id) { return weatherLocationById(id) == nullptr; }),
                                  settings_.favorites.end());
        if (settings_.favorites.empty()) settings_.favorites.push_back("santiago");
        if (!weatherLocationById(settings_.activeCityId)) settings_.activeCityId = settings_.favorites.front();
        const auto cache = root.find("cache");
        if (cache != root.end()) {
            for (auto it = cache->begin(); it != cache->end(); ++it) {
                if (auto value = deserialize(it.key(), it.value())) cache_[it.key()] = *value;
            }
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool SettingsStore::save() const {
    try {
        const auto parent = std::filesystem::path(path_).parent_path();
        if (!parent.empty()) std::filesystem::create_directories(parent);
        nlohmann::json cache = nlohmann::json::object();
        for (const auto& [id, data] : cache_) cache[id] = serialize(data);
        std::ofstream output(path_);
        if (!output) return false;
        output << nlohmann::json{
            {"activeCityId", settings_.activeCityId},
            {"favorites", settings_.favorites},
            {"cache",
             cache}}.dump(2);
        return static_cast<bool>(output);
    } catch (const std::exception&) {
        return false;
    }
}

void SettingsStore::setActiveCity(const std::string& id) {
    if (weatherLocationById(id)) settings_.activeCityId = id;
}
bool SettingsStore::isFavorite(const std::string& id) const {
    return std::find(settings_.favorites.begin(), settings_.favorites.end(), id) != settings_.favorites.end();
}
bool SettingsStore::addFavorite(const std::string& id) {
    if (!weatherLocationById(id) || isFavorite(id)) return false;
    settings_.favorites.push_back(id);
    return true;
}
bool SettingsStore::removeFavorite(const std::string& id) {
    if (settings_.favorites.size() <= 1) return false;
    auto it = std::find(settings_.favorites.begin(), settings_.favorites.end(), id);
    if (it == settings_.favorites.end()) return false;
    settings_.favorites.erase(it);
    if (settings_.activeCityId == id) settings_.activeCityId = settings_.favorites.front();
    return true;
}
bool SettingsStore::moveFavorite(const std::string& id, int direction) {
    auto it = std::find(settings_.favorites.begin(), settings_.favorites.end(), id);
    if (it == settings_.favorites.end()) return false;
    const auto index = std::distance(settings_.favorites.begin(), it);
    const auto destination = index + direction;
    if (destination < 0 || destination >= static_cast<long>(settings_.favorites.size())) return false;
    std::iter_swap(it, settings_.favorites.begin() + destination);
    return true;
}
void SettingsStore::putCache(const WeatherData& data) {
    if (data.status == WeatherStatus::Ready) cache_[data.location.id] = data;
}
std::optional<WeatherData> SettingsStore::cached(const std::string& cityId, long long now,
                                                 long long maxAgeSeconds) const {
    const auto it = cache_.find(cityId);
    if (it == cache_.end() || now - it->second.updatedAt > maxAgeSeconds) return std::nullopt;
    return it->second;
}
