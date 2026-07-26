#include "weather/weather_client.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace {
constexpr std::array<WeatherLocation, 4> kLocations{{
    {"santiago", "Santiago", "Chile", -33.4489, -70.6693},
    {"valparaiso", "Valparaiso", "Chile", -33.0472, -71.6127},
    {"puerto-montt", "Puerto Montt", "Chile", -41.4693, -72.9424},
    {"concepcion", "Concepcion", "Chile", -36.8201, -73.0444},
}};

size_t appendResponse(char* data, size_t size, size_t count, void* target) {
    auto* response = static_cast<std::string*>(target);
    response->append(data, size * count);
    return size * count;
}

int rounded(const nlohmann::json& value) { return static_cast<int>(std::lround(value.get<double>())); }
}  // namespace

WeatherClient::WeatherClient() { curl_global_init(CURL_GLOBAL_DEFAULT); }

WeatherClient::~WeatherClient() {
    for (auto& worker : workers_) {
        if (worker.joinable()) worker.join();
    }
    curl_global_cleanup();
}

void WeatherClient::fetch(const WeatherLocation& location) {
    workers_.emplace_back([this, location] {
        WeatherData data = request(location);
        std::lock_guard<std::mutex> lock(mutex_);
        results_.push(std::move(data));
    });
}

std::optional<WeatherData> WeatherClient::takeResult() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (results_.empty()) return std::nullopt;
    WeatherData result = std::move(results_.front());
    results_.pop();
    return result;
}

WeatherData WeatherClient::request(const WeatherLocation& location) {
    WeatherData result{};
    result.location = location;
    result.status = WeatherStatus::Error;
    CURL* curl = curl_easy_init();
    if (!curl) { result.error = "No se pudo iniciar la red"; return result; }

    std::ostringstream url;
    url << "https://api.open-meteo.com/v1/forecast?latitude=" << location.latitude
        << "&longitude=" << location.longitude
        << "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m"
        << "&daily=weather_code,temperature_2m_max,temperature_2m_min"
        << "&forecast_days=5&timezone=auto";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    const CURLcode code = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    if (code != CURLE_OK) { result.error = curl_easy_strerror(code); return result; }
    if (httpCode != 200) { result.error = "El servicio meteorologico no respondio"; return result; }

    try {
        const auto json = nlohmann::json::parse(response);
        const auto& current = json.at("current");
        const auto& daily = json.at("daily");
        const auto& dates = daily.at("time");
        const auto& codes = daily.at("weather_code");
        const auto& maximums = daily.at("temperature_2m_max");
        const auto& minimums = daily.at("temperature_2m_min");
        if (dates.size() < 5 || codes.size() < 5 || maximums.size() < 5 || minimums.size() < 5) {
            throw std::runtime_error("Prevision incompleta");
        }
        result.weatherCode = current.at("weather_code").get<int>();
        result.temperature = rounded(current.at("temperature_2m"));
        result.humidity = current.at("relative_humidity_2m").get<int>();
        result.windSpeed = rounded(current.at("wind_speed_10m"));
        for (size_t index = 0; index < result.daily.size(); ++index) {
            result.daily[index] = {dates.at(index).get<std::string>(), codes.at(index).get<int>(),
                                   rounded(maximums.at(index)), rounded(minimums.at(index))};
        }
        result.status = WeatherStatus::Ready;
    } catch (const std::exception& error) {
        result.error = error.what();
    }
    return result;
}

const std::array<WeatherLocation, 4>& weatherLocations() { return kLocations; }

const char* weatherDescription(int code) {
    if (code == 0) return "Despejado";
    if (code <= 2) return "Parcialmente nublado";
    if (code == 3) return "Nublado";
    if (code == 45 || code == 48) return "Niebla";
    if (code >= 51 && code <= 57) return "Llovizna";
    if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82)) return "Lluvia";
    if (code >= 71 && code <= 77) return "Nieve";
    if (code >= 95) return "Tormenta";
    return "Sin datos";
}

const char* weatherSymbol(int code) {
    if (code == 0) return "*";
    if (code <= 2) return "o~";
    if (code == 3) return "~";
    if (code == 45 || code == 48) return "=";
    if (code >= 51 && code <= 82) return "///";
    if (code >= 95) return "!";
    return "+";
}

bool isStormyOrRainy(int code) {
    return (code >= 51 && code <= 67) || (code >= 80 && code <= 99) || code == 3;
}
