#include "weather/weather_client.h"

#include <chrono>
#include <cmath>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace {
int rounded(const nlohmann::json& value) { return static_cast<int>(std::lround(value.get<double>())); }
long long nowSeconds() { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
}

WeatherData WeatherClient::parseResponse(const WeatherLocation& location, const std::string& response) {
    WeatherData result{}; result.location = location; result.status = WeatherStatus::Error;
    try {
        const auto json = nlohmann::json::parse(response); const auto& current = json.at("current"); const auto& daily = json.at("daily");
        const auto& dates = daily.at("time"); const auto& codes = daily.at("weather_code"); const auto& maximums = daily.at("temperature_2m_max"); const auto& minimums = daily.at("temperature_2m_min"); const auto& probabilities = daily.at("precipitation_probability_max"); const auto& precipitation = daily.at("precipitation_sum");
        if (dates.size() < 5 || codes.size() < 5 || maximums.size() < 5 || minimums.size() < 5 || probabilities.size() < 5 || precipitation.size() < 5) throw std::runtime_error("Previsión incompleta");
        result.weatherCode = current.at("weather_code").get<int>(); result.temperature = rounded(current.at("temperature_2m")); result.apparentTemperature = rounded(current.at("apparent_temperature")); result.humidity = current.at("relative_humidity_2m").get<int>(); result.windSpeed = rounded(current.at("wind_speed_10m")); result.precipitation = current.value("precipitation", 0.0); result.precipitationProbability = probabilities.at(0).get<int>();
        for (size_t index = 0; index < result.daily.size(); ++index) result.daily[index] = {dates.at(index).get<std::string>(), codes.at(index).get<int>(), rounded(maximums.at(index)), rounded(minimums.at(index)), probabilities.at(index).get<int>(), precipitation.at(index).get<double>()};
        result.updatedAt = nowSeconds(); result.status = WeatherStatus::Ready;
    } catch (const std::exception& error) { result.error = error.what(); }
    return result;
}
