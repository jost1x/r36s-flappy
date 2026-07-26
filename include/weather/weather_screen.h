#pragma once

#include "weather_client.h"
#include <lvgl.h>
#include <array>
#include <optional>

class WeatherScreen {
public:
    explicit WeatherScreen(WeatherClient& client);
    ~WeatherScreen() = default;

    void show();
    void handleAction(int action);
    void update();

private:
    void build();
    void selectCity(int index, bool fetch = true);
    void applyWeather(const WeatherData& data);
    void updateStaticData();
    void updateCityIndicator(bool animate);
    void refreshTheme(int weatherCode);
    void rebuildWeatherIcon(int weatherCode);
    void setLoading(bool loading);
    static const lv_image_dsc_t* weatherImage(int weatherCode);
    void setModalVisible(bool visible);
    void updateForecastSelection();
    void showNotice(const char* text);
    static void cityButtonEvent(lv_event_t* event);

    WeatherClient& client_;
    int cityIndex_ = 0;
    int dayIndex_ = 0;
    bool modalVisible_ = false;
    bool hasCurrentData_ = false;
    std::optional<WeatherData> currentData_;

    lv_obj_t* root_ = nullptr;
    lv_obj_t* card_ = nullptr;
    lv_obj_t* cityLabel_ = nullptr;
    lv_obj_t* countryLabel_ = nullptr;
    std::array<lv_obj_t*, 4> cityIndicators_{};
    lv_obj_t* weatherVisual_ = nullptr;
    lv_obj_t* conditionIcon_ = nullptr;
    lv_obj_t* temperatureLabel_ = nullptr;
    lv_obj_t* conditionLabel_ = nullptr;
    lv_obj_t* rangeLabel_ = nullptr;
    lv_obj_t* humidityValue_ = nullptr;
    lv_obj_t* windValue_ = nullptr;
    std::array<lv_obj_t*, 2> statPanels_{};
    lv_obj_t* noticeLabel_ = nullptr;
    lv_obj_t* skeletonLayer_ = nullptr;
    std::array<lv_obj_t*, 13> skeletonBlocks_{};
    lv_obj_t* modalBackdrop_ = nullptr;
    lv_obj_t* modal_ = nullptr;
    std::array<lv_obj_t*, 4> modalRows_{};
    std::array<lv_obj_t*, 5> dayCards_{};
    std::array<lv_obj_t*, 5> dayLabels_{};
    std::array<lv_obj_t*, 5> dayTemperatureLabels_{};
    std::array<lv_obj_t*, 5> dayIcons_{};
};
