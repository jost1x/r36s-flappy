#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "weather/settings_store.h"
#include "weather/weather_client.h"

void runIconRenderTest();

namespace {
std::string response() {
    return R"({"current":{"temperature_2m":18.4,"apparent_temperature":17.6,"relative_humidity_2m":67,"weather_code":61,"wind_speed_10m":12.3,"precipitation":1.2},"daily":{"time":["2026-07-25","2026-07-26","2026-07-27","2026-07-28","2026-07-29"],"weather_code":[61,3,0,2,80],"temperature_2m_max":[20,21,22,23,19],"temperature_2m_min":[8,9,10,11,7],"precipitation_probability_max":[75,20,5,10,80],"precipitation_sum":[2.5,0,0,0.1,4.2]}})";
}
}  // namespace

int main() {
    const auto* location = weatherLocationById("santiago");
    assert(location);
    const auto parsed = WeatherClient::parseResponse(*location, response());
    assert(parsed.status == WeatherStatus::Ready);
    assert(parsed.temperature == 18 && parsed.apparentTemperature == 18);
    assert(parsed.precipitationProbability == 75 && parsed.daily[4].precipitation == 4.2);
    assert(WeatherClient::parseResponse(*location, "{}").status == WeatherStatus::Error);
    assert(std::string(weatherDescription(61)) == "Lluvia");

    const auto path = (std::filesystem::temp_directory_path() / "r36s-weather-test.json").string();
    std::filesystem::remove(path);
    SettingsStore store(path);
    assert(store.addFavorite("temuco"));
    assert(store.moveFavorite("temuco", -1));
    store.setActiveCity("temuco");
    auto cache = parsed;
    cache.updatedAt = 1'000;
    store.putCache(cache);
    assert(store.save());

    SettingsStore restored(path);
    assert(restored.load());
    assert(restored.settings().activeCityId == "temuco");
    assert(restored.isFavorite("temuco"));
    assert(restored.cached("santiago", 1'000 + 60, 24 * 60 * 60));
    assert(!restored.cached("santiago", 1'000 + 24 * 60 * 60 + 1, 24 * 60 * 60));
    std::filesystem::remove(path);
    runIconRenderTest();
    std::cout << "weather core tests passed\n";
}
