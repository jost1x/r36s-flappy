#pragma once

#include "weather_client.h"
#include "settings_store.h"
#include <lvgl.h>
#include <array>
#include <optional>
#include <vector>

class WeatherScreen {
public:
    WeatherScreen(WeatherClient& client, SettingsStore& settings);
    ~WeatherScreen() = default;

    void show();
    void handleAction(int action);
    void update();

private:
    void build();
    void selectCity(int index, bool fetch = true);
    void selectCityId(const std::string& id, bool fetch = true);
    void refresh(bool manual = false);
    void applyWeather(const WeatherData& data);
    void updateStaticData();
    void updateCityIndicator(bool animate);
    void refreshTheme(int weatherCode);
    void rebuildWeatherIcon(int weatherCode);
    void setLoading(bool loading);
    static const lv_image_dsc_t* weatherImage(int weatherCode);
    void setModalVisible(bool visible);
    void updateModalRows();
    std::vector<int> selectableCities() const;
    long long nowSeconds() const;
    void updateForecastSelection();
    void updateDetail();
    void showNotice(const char* text);
    void setDetailVisible(bool visible);
    static void cityButtonEvent(lv_event_t* event);

    WeatherClient& client_;
    SettingsStore& settings_;
    int cityIndex_ = 0;
    int dayIndex_ = 0;
    bool modalVisible_ = false;
    bool detailVisible_ = false;
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
    lv_obj_t* feelsValue_ = nullptr;
    lv_obj_t* rainValue_ = nullptr;
    std::array<lv_obj_t*, 4> statPanels_{};
    lv_obj_t* updatedLabel_ = nullptr;
    lv_obj_t* noticeLabel_ = nullptr;
    lv_obj_t* skeletonLayer_ = nullptr;
    std::array<lv_obj_t*, 13> skeletonBlocks_{};
    lv_obj_t* modalBackdrop_ = nullptr;
    lv_obj_t* modal_ = nullptr;
    std::array<lv_obj_t*, 4> modalRows_{};
    std::array<lv_obj_t*, 4> modalRowNames_{};
    std::array<lv_obj_t*, 4> modalRowDetails_{};
    std::array<int, 4> modalRowCityIndices_{{-1, -1, -1, -1}};
    lv_obj_t* modalTitle_ = nullptr;
    lv_obj_t* modalPageLabel_ = nullptr;
    lv_obj_t* detail_ = nullptr;
    lv_obj_t* detailCityLabel_ = nullptr;
    lv_obj_t* detailDayLabel_ = nullptr;
    lv_obj_t* detailIcon_ = nullptr;
    lv_obj_t* detailConditionLabel_ = nullptr;
    lv_obj_t* detailRangeLabel_ = nullptr;
    lv_obj_t* detailRainLabel_ = nullptr;
    lv_obj_t* detailPrecipitationLabel_ = nullptr;
    bool showingCatalog_ = false;
    std::array<lv_obj_t*, 5> dayCards_{};
    std::array<lv_obj_t*, 5> dayLabels_{};
    std::array<lv_obj_t*, 5> dayTemperatureLabels_{};
    std::array<lv_obj_t*, 5> dayIcons_{};
    unsigned long long latestRequestId_ = 0;
    long long lastRefreshAt_ = 0;
};
