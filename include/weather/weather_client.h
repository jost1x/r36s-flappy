#pragma once

#include <array>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

enum class WeatherStatus { Loading, Ready, Error };

struct WeatherLocation {
    const char* id;
    const char* name;
    const char* country;
    double latitude;
    double longitude;
};

struct DailyForecast {
    std::string date;
    int weatherCode = -1;
    int maximum = 0;
    int minimum = 0;
};

struct WeatherData {
    WeatherLocation location;
    WeatherStatus status = WeatherStatus::Loading;
    int weatherCode = -1;
    int temperature = 0;
    int humidity = 0;
    int windSpeed = 0;
    std::array<DailyForecast, 5> daily{};
    std::string error;
};

class WeatherClient {
public:
    using ResultHandler = std::function<void(WeatherData)>;

    WeatherClient();
    ~WeatherClient();
    WeatherClient(const WeatherClient&) = delete;
    WeatherClient& operator=(const WeatherClient&) = delete;

    void fetch(const WeatherLocation& location);
    std::optional<WeatherData> takeResult();

private:
    static WeatherData request(const WeatherLocation& location);
    std::mutex mutex_;
    std::queue<WeatherData> results_;
    std::vector<std::thread> workers_;
};

const std::array<WeatherLocation, 4>& weatherLocations();
const char* weatherDescription(int weatherCode);
const char* weatherSymbol(int weatherCode);
bool isStormyOrRainy(int weatherCode);
