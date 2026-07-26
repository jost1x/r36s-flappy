#include "weather/weather_client.h"

#include <curl/curl.h>

#include <sstream>

namespace {
size_t appendResponse(char* data, size_t size, size_t count, void* target) {
    static_cast<std::string*>(target)->append(data, size * count);
    return size * count;
}
}

WeatherClient::WeatherClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    worker_ = std::thread(&WeatherClient::workerLoop, this);
}

WeatherClient::~WeatherClient() {
    { std::lock_guard<std::mutex> lock(mutex_); stopping_ = true; }
    pendingChanged_.notify_one();
    if (worker_.joinable()) worker_.join();
    curl_global_cleanup();
}

void WeatherClient::fetch(const WeatherLocation& location) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto id = ++nextRequestId_;
    latestRequest_[location.id] = id;
    pending_[location.id] = {location, id};
    pendingChanged_.notify_one();
}

void WeatherClient::workerLoop() {
    for (;;) {
        PendingRequest pending{};
        { std::unique_lock<std::mutex> lock(mutex_); pendingChanged_.wait(lock, [this] { return stopping_ || !pending_.empty(); }); if (stopping_) return; auto it = pending_.begin(); pending = it->second; pending_.erase(it); }
        WeatherData result = request(pending.location);
        result.requestId = pending.id;
        std::lock_guard<std::mutex> lock(mutex_);
        if (latestRequest_[pending.location.id] == pending.id) results_.push_back(std::move(result));
    }
}

std::optional<WeatherData> WeatherClient::takeResult() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (results_.empty()) return std::nullopt;
    WeatherData result = std::move(results_.front());
    results_.erase(results_.begin());
    return result;
}

WeatherData WeatherClient::request(const WeatherLocation& location) {
    WeatherData failed{}; failed.location = location; failed.status = WeatherStatus::Error;
    for (int attempt = 0; attempt < 2; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) { failed.error = "No se pudo iniciar la red"; return failed; }
        std::ostringstream url;
        url << "https://api.open-meteo.com/v1/forecast?latitude=" << location.latitude << "&longitude=" << location.longitude << "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m,precipitation&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,precipitation_sum&forecast_days=5&timezone=auto";
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str()); curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendResponse); curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L); curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 4L); curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        const CURLcode code = curl_easy_perform(curl); long httpCode = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode); curl_easy_cleanup(curl);
        if (code == CURLE_OK && httpCode == 200) return parseResponse(location, response);
        failed.error = code != CURLE_OK ? curl_easy_strerror(code) : "El servicio meteorológico no respondió";
    }
    return failed;
}
