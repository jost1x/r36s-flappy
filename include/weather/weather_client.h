#pragma once

#include <array>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

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
    int precipitationProbability = 0;
    double precipitation = 0.0;
};

struct WeatherData {
    WeatherLocation location;
    WeatherStatus status = WeatherStatus::Loading;
    int weatherCode = -1;
    int temperature = 0;
    int humidity = 0;
    int windSpeed = 0;
    int apparentTemperature = 0;
    int precipitationProbability = 0;
    double precipitation = 0.0;
    std::array<DailyForecast, 5> daily{};
    std::string error;
    long long updatedAt = 0;
    unsigned long long requestId = 0;
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

    // Pure parsing entry point used by tests and by the network worker.
    static WeatherData parseResponse(const WeatherLocation& location, const std::string& response);

private:
    struct PendingRequest { WeatherLocation location; unsigned long long id; };
    static WeatherData request(const WeatherLocation& location);
    void workerLoop();
    std::mutex mutex_;
    std::condition_variable pendingChanged_;
    std::vector<WeatherData> results_;
    std::unordered_map<std::string, PendingRequest> pending_;
    std::unordered_map<std::string, unsigned long long> latestRequest_;
    std::thread worker_;
    bool stopping_ = false;
    unsigned long long nextRequestId_ = 1;
};

const std::array<WeatherLocation, 18>& weatherLocations();
const WeatherLocation* weatherLocationById(const std::string& id);
const char* weatherDescription(int weatherCode);
const char* weatherSymbol(int weatherCode);
bool isStormyOrRainy(int weatherCode);
