#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "weather/weather_icons.h"
#include "weather/weather_screen.h"

namespace {
constexpr lv_color_t kWhite = LV_COLOR_MAKE(245, 248, 252);
constexpr lv_color_t kBlue = LV_COLOR_MAKE(40, 157, 221);
constexpr lv_color_t kBlueDark = LV_COLOR_MAKE(40, 131, 183);
constexpr lv_color_t kSlate = LV_COLOR_MAKE(61, 75, 92);
constexpr lv_color_t kSlateDark = LV_COLOR_MAKE(51, 64, 79);
constexpr int kIndicatorWidth = 6;
constexpr int kActiveIndicatorWidth = 20;
constexpr int kIndicatorGap = 6;

const char* shortDay(const DailyForecast& forecast, size_t index) {
    if (index == 0) return "Hoy";
    std::tm date{};
    std::istringstream input(forecast.date);
    input >> std::get_time(&date, "%Y-%m-%d");
    if (input.fail()) return "--";
    std::mktime(&date);
    constexpr std::array<const char*, 7> names{{"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"}};
    return names[static_cast<size_t>(date.tm_wday)];
}

void setObjectBackgroundOpacity(void* object, int32_t value) {
    lv_obj_set_style_bg_opa(static_cast<lv_obj_t*>(object), static_cast<lv_opa_t>(value), LV_PART_MAIN);
}

void setObjectX(void* object, int32_t value) { lv_obj_set_x(static_cast<lv_obj_t*>(object), value); }
void setObjectWidth(void* object, int32_t value) { lv_obj_set_width(static_cast<lv_obj_t*>(object), value); }

void animateObjectValue(lv_obj_t* object, lv_anim_exec_xcb_t callback, int from, int to, bool animate) {
    lv_anim_delete(object, callback);
    if (!animate || from == to) {
        callback(object, to);
        return;
    }
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, object);
    lv_anim_set_exec_cb(&animation, callback);
    lv_anim_set_values(&animation, from, to);
    lv_anim_set_duration(&animation, 180);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_start(&animation);
}
}  // namespace

void WeatherScreen::updateStaticData() {
    const auto& location = weatherLocations()[static_cast<size_t>(cityIndex_)];
    lv_label_set_text(cityLabel_, location.name);
    lv_label_set_text(countryLabel_, location.country);
    updateCityIndicator(true);
}

void WeatherScreen::updateCityIndicator(bool animate) {
    int x = 514;
    const auto& favorites = settings_.settings().favorites;
    const auto& cityId = weatherLocations()[static_cast<size_t>(cityIndex_)].id;
    const auto favorite = std::find(favorites.begin(), favorites.end(), cityId);
    const int activeIndex =
        favorite != favorites.end()
            ? static_cast<int>(std::distance(favorites.begin(), favorite)) * static_cast<int>(cityIndicators_.size()) /
                  static_cast<int>(favorites.size())
            : cityIndex_ * static_cast<int>(cityIndicators_.size()) / static_cast<int>(weatherLocations().size());
    for (size_t index = 0; index < cityIndicators_.size(); ++index) {
        auto* indicator = cityIndicators_[index];
        const bool active = static_cast<int>(index) == activeIndex;
        const int width = active ? kActiveIndicatorWidth : kIndicatorWidth;
        animateObjectValue(indicator, setObjectX, lv_obj_get_x(indicator), x, animate);
        animateObjectValue(indicator, setObjectWidth, lv_obj_get_width(indicator), width, animate);
        animateObjectValue(indicator, setObjectBackgroundOpacity, lv_obj_get_style_bg_opa(indicator, LV_PART_MAIN),
                           active ? LV_OPA_COVER : LV_OPA_50, animate);
        x += width + kIndicatorGap;
    }
}

void WeatherScreen::setLoading(bool loading) {
    if (loading)
        lv_obj_clear_flag(skeletonLayer_, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(skeletonLayer_, LV_OBJ_FLAG_HIDDEN);
    for (size_t index = 0; index < skeletonBlocks_.size(); ++index) {
        auto* block = skeletonBlocks_[index];
        lv_anim_delete(block, setObjectBackgroundOpacity);
        if (!loading) continue;
        lv_anim_t pulse;
        lv_anim_init(&pulse);
        lv_anim_set_var(&pulse, block);
        lv_anim_set_exec_cb(&pulse, setObjectBackgroundOpacity);
        lv_anim_set_values(&pulse, LV_OPA_10, LV_OPA_30);
        lv_anim_set_duration(&pulse, 650);
        lv_anim_set_delay(&pulse, static_cast<uint32_t>(index % 3) * 90);
        lv_anim_set_playback_duration(&pulse, 650);
        lv_anim_set_repeat_count(&pulse, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_path_cb(&pulse, lv_anim_path_ease_in_out);
        lv_anim_start(&pulse);
    }
}

void WeatherScreen::applyWeather(const WeatherData& data) {
    setLoading(false);
    currentData_ = data;
    hasCurrentData_ = true;
    const auto setText = [](lv_obj_t* label, const std::string& value) { lv_label_set_text(label, value.c_str()); };
    setText(temperatureLabel_, fmt::format("{}°", data.temperature));
    lv_label_set_text(conditionLabel_, weatherDescription(data.weatherCode));
    setText(rangeLabel_, fmt::format("Max {}° / Min {}°", data.daily[0].maximum, data.daily[0].minimum));
    setText(humidityValue_, fmt::format("{}%", data.humidity));
    setText(windValue_, fmt::format("{} km/h", data.windSpeed));
    setText(feelsValue_, fmt::format("{}°", data.apparentTemperature));
    setText(rainValue_, fmt::format("{}%", data.precipitationProbability));
    for (size_t index = 0; index < data.daily.size(); ++index) {
        lv_label_set_text(dayLabels_[index], shortDay(data.daily[index], index));
        lv_image_set_src(dayIcons_[index], weatherImage(data.daily[index].weatherCode));
        lv_obj_invalidate(dayIcons_[index]);
        setText(dayTemperatureLabels_[index],
                fmt::format("{}° {}°", data.daily[index].maximum, data.daily[index].minimum));
    }
    refreshTheme(data.weatherCode);
    rebuildWeatherIcon(data.weatherCode);
    const long long age = std::max(0LL, nowSeconds() - data.updatedAt);
    setText(updatedLabel_, fmt::format("Actualizado hace {} min", age / 60));
    settings_.putCache(data);
    settings_.save();
    showNotice("");
    if (detailVisible_) updateDetail();
}

void WeatherScreen::rebuildWeatherIcon(int weatherCode) {
    lv_image_set_src(weatherVisual_, weatherImage(weatherCode));
    lv_obj_invalidate(weatherVisual_);
}

const lv_image_dsc_t* WeatherScreen::weatherImage(int weatherCode) {
    if (weatherCode == 0) return &sun;
    if (weatherCode == 1 || weatherCode == 2) return &cloud_sun;
    if (weatherCode == 3) return &cloud;
    if (weatherCode == 45 || weatherCode == 48) return &cloud_fog;
    if ((weatherCode >= 51 && weatherCode <= 67) || (weatherCode >= 80 && weatherCode <= 82)) return &cloud_rain;
    if (weatherCode >= 71 && weatherCode <= 77) return &cloud_snow;
    if (weatherCode >= 95) return &cloud_lightning;
    return &cloud;
}

void WeatherScreen::refreshTheme(int weatherCode) {
    const bool rainy = isStormyOrRainy(weatherCode);
    const lv_color_t base = rainy ? kSlate : kBlue;
    lv_obj_set_style_bg_color(root_, rainy ? kSlateDark : kBlueDark, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(root_, base, LV_PART_MAIN);
    lv_obj_set_style_bg_color(card_, base, LV_PART_MAIN);
    lv_obj_set_style_bg_color(skeletonLayer_, base, LV_PART_MAIN);
    lv_color_t panelColor = LV_COLOR_MAKE(98, 183, 224);
    if (rainy) panelColor = LV_COLOR_MAKE(104, 121, 139);
    for (auto* day : dayCards_) lv_obj_set_style_bg_color(day, panelColor, LV_PART_MAIN);
    for (auto* panel : statPanels_) {
        lv_obj_set_style_bg_color(panel, panelColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(panel, LV_OPA_60, LV_PART_MAIN);
    }
}

void WeatherScreen::updateForecastSelection() {
    for (size_t index = 0; index < dayCards_.size(); ++index) {
        lv_obj_set_style_border_width(dayCards_[index], static_cast<int>(index) == dayIndex_ ? 2 : 0, LV_PART_MAIN);
        lv_obj_set_style_border_color(dayCards_[index], kWhite, LV_PART_MAIN);
    }
}

void WeatherScreen::updateDetail() {
    const auto& location = weatherLocations()[static_cast<size_t>(cityIndex_)];
    lv_label_set_text(detailCityLabel_, location.name);
    if (!currentData_) {
        lv_label_set_text(detailDayLabel_, "Sin datos disponibles");
        lv_label_set_text(detailConditionLabel_, "Sin pronóstico");
        return;
    }

    const auto& forecast = currentData_->daily[static_cast<size_t>(dayIndex_)];
    const auto day = fmt::format("{} · {}", shortDay(forecast, static_cast<size_t>(dayIndex_)), forecast.date);
    lv_label_set_text(detailDayLabel_, day.c_str());
    lv_image_set_src(detailIcon_, weatherImage(forecast.weatherCode));
    lv_obj_invalidate(detailIcon_);
    lv_label_set_text(detailConditionLabel_, weatherDescription(forecast.weatherCode));
    const auto range = fmt::format("Máxima {}° / Mínima {}°", forecast.maximum, forecast.minimum);
    lv_label_set_text(detailRangeLabel_, range.c_str());
    const auto probability = fmt::format("Probabilidad de lluvia: {}%", forecast.precipitationProbability);
    lv_label_set_text(detailRainLabel_, probability.c_str());
    const auto precipitation = fmt::format("Precipitación esperada: {:.1f} mm", forecast.precipitation);
    lv_label_set_text(detailPrecipitationLabel_, precipitation.c_str());
}

void WeatherScreen::showNotice(const char* value) { lv_label_set_text(noticeLabel_, value); }
