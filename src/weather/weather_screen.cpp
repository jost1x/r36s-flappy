#include "weather/weather_screen.h"

#include "weather/weather_icons.h"

#include "input_manager.h"
#include <lvgl.h>

#include <array>
#include <cstdio>

namespace {
constexpr lv_color_t kWhite = LV_COLOR_MAKE(245, 248, 252);
constexpr lv_color_t kBlue = LV_COLOR_MAKE(40, 157, 221);
constexpr lv_color_t kBlueDark = LV_COLOR_MAKE(40, 131, 183);
constexpr lv_color_t kSlate = LV_COLOR_MAKE(61, 75, 92);
constexpr lv_color_t kSlateDark = LV_COLOR_MAKE(51, 64, 79);
constexpr std::array<const char*, 5> kDayNames{{"Hoy", "Man", "Mie", "Jue", "Vie"}};
constexpr int kIndicatorHeight = 6;
constexpr int kIndicatorWidth = 6;
constexpr int kActiveIndicatorWidth = 20;
constexpr int kIndicatorGap = 6;
constexpr int kIndicatorY = 35;

enum class ControlIcon { Left, Right, Up, Down, Confirm };

// The 10 px glyphs are centred in the 22 x 20 keys, leaving a calm inner margin.
constexpr lv_point_precise_t kLeftHead[] = {{11, 6}, {7, 10}, {11, 14}};
constexpr lv_point_precise_t kLeftShaft[] = {{7, 10}, {15, 10}};
constexpr lv_point_precise_t kRightHead[] = {{11, 6}, {15, 10}, {11, 14}};
constexpr lv_point_precise_t kRightShaft[] = {{7, 10}, {15, 10}};
constexpr lv_point_precise_t kUpHead[] = {{7, 11}, {11, 6}, {15, 11}};
constexpr lv_point_precise_t kUpShaft[] = {{11, 6}, {11, 15}};
constexpr lv_point_precise_t kDownHead[] = {{7, 9}, {11, 14}, {15, 9}};
constexpr lv_point_precise_t kDownShaft[] = {{11, 5}, {11, 14}};
constexpr lv_point_precise_t kConfirmBody[] = {{7, 15}, {11, 5}, {15, 15}};
constexpr lv_point_precise_t kConfirmBar[] = {{9, 11}, {13, 11}};

void drawIconLine(lv_obj_t* parent, const lv_point_precise_t points[], size_t count) {
    auto* line = lv_line_create(parent);
    lv_line_set_points(line, points, count);
    lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(line, kWhite, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN);
}

void drawControlIcon(lv_obj_t* parent, ControlIcon icon) {
    switch (icon) {
        case ControlIcon::Left:
            drawIconLine(parent, kLeftHead, 3);
            drawIconLine(parent, kLeftShaft, 2);
            break;
        case ControlIcon::Right:
            drawIconLine(parent, kRightHead, 3);
            drawIconLine(parent, kRightShaft, 2);
            break;
        case ControlIcon::Up:
            drawIconLine(parent, kUpHead, 3);
            drawIconLine(parent, kUpShaft, 2);
            break;
        case ControlIcon::Down:
            drawIconLine(parent, kDownHead, 3);
            drawIconLine(parent, kDownShaft, 2);
            break;
        case ControlIcon::Confirm:
            drawIconLine(parent, kConfirmBody, 3);
            drawIconLine(parent, kConfirmBar, 2);
            break;
    }
}

void setObjectBackgroundOpacity(void* object, int32_t value) {
    lv_obj_set_style_bg_opa(static_cast<lv_obj_t*>(object), static_cast<lv_opa_t>(value), LV_PART_MAIN);
}

void setObjectX(void* object, int32_t value) {
    lv_obj_set_x(static_cast<lv_obj_t*>(object), value);
}

void setObjectWidth(void* object, int32_t value) {
    lv_obj_set_width(static_cast<lv_obj_t*>(object), value);
}

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

void rect(lv_obj_t* object, int x, int y, int width, int height) {
    lv_obj_set_pos(object, x, y);
    lv_obj_set_size(object, width, height);
}

void labelStyle(lv_obj_t* label, lv_color_t color, lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, align, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN);
}

lv_obj_t* text(lv_obj_t* parent, const char* value, int x, int y, int width, int height, lv_color_t color,
               lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
    auto* label = lv_label_create(parent);
    lv_label_set_text(label, value);
    rect(label, x, y, width, height);
    labelStyle(label, color, align);
    return label;
}

const char* shortDay(int index) {
    if (index == 0) return "Hoy";
    // The API date is stable ISO-8601; local weekday conversion is intentionally not needed for this compact UI.
    return kDayNames[static_cast<size_t>(index)];
}
}  // namespace

WeatherScreen::WeatherScreen(WeatherClient& client) : client_(client) { build(); }

void WeatherScreen::show() {
    lv_scr_load(root_);
    selectCity(0);
}

void WeatherScreen::build() {
    root_ = lv_obj_create(nullptr);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root_, kBlueDark, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(root_, kBlue, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(root_, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(root_, 0, LV_PART_MAIN);

    card_ = lv_obj_create(root_);
    rect(card_, 16, 16, 608, 448);
    lv_obj_clear_flag(card_, LV_OBJ_FLAG_SCROLLABLE);
    // The 640x480 LVGL display is the real device screen, not a mockup card.
    // Keep this transparent layout parent only to preserve the screen spacing.
    lv_obj_set_style_radius(card_, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(card_, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card_, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card_, 0, LV_PART_MAIN);

    cityLabel_ = text(card_, "Santiago", 34, 24, 220, 24, kWhite);
    countryLabel_ = text(card_, "Chile", 34, 45, 220, 20, LV_COLOR_MAKE(210, 238, 255));
    for (auto& indicator : cityIndicators_) {
        indicator = lv_obj_create(card_);
        rect(indicator, 0, kIndicatorY, kIndicatorWidth, kIndicatorHeight);
        lv_obj_clear_flag(indicator, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(indicator, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_color(indicator, kWhite, LV_PART_MAIN);
        lv_obj_set_style_border_width(indicator, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(indicator, 0, LV_PART_MAIN);
    }
    updateCityIndicator(false);
    lv_obj_set_style_text_font(cityLabel_, &lv_font_montserrat_20, LV_PART_MAIN);

    weatherVisual_ = lv_image_create(card_);
    rect(weatherVisual_, 40, 118, 100, 100);
    lv_obj_set_style_image_recolor(weatherVisual_, kWhite, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(weatherVisual_, LV_OPA_COVER, LV_PART_MAIN);
    conditionIcon_ = text(card_, "", 0, 0, 0, 0, kWhite);
    lv_obj_add_flag(conditionIcon_, LV_OBJ_FLAG_HIDDEN);
    temperatureLabel_ = text(card_, "--°", 164, 96, 240, 62, kWhite);
    conditionLabel_ = text(card_, "Cargando...", 164, 168, 280, 28, kWhite);
    rangeLabel_ = text(card_, "Max --° / Min --°", 164, 196, 280, 24, LV_COLOR_MAKE(220, 240, 250));
    lv_obj_set_style_text_font(temperatureLabel_, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_font(conditionLabel_, &lv_font_montserrat_20, LV_PART_MAIN);
    text(card_, "Hoy", 70, 218, 75, 20, kWhite, LV_TEXT_ALIGN_CENTER);

    auto* humidity = lv_obj_create(card_);
    statPanels_[0] = humidity;
    rect(humidity, 164, 224, 124, 60);
    lv_obj_set_style_radius(humidity, 14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(humidity, LV_COLOR_MAKE(98, 183, 224), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(humidity, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(humidity, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(humidity, 0, LV_PART_MAIN);
    auto* humidityIcon = lv_image_create(humidity);
    rect(humidityIcon, 11, 20, 20, 20);
    lv_image_set_src(humidityIcon, &droplets);
    lv_image_set_scale(humidityIcon, 53);
    lv_obj_set_style_image_recolor(humidityIcon, kWhite, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(humidityIcon, LV_OPA_COVER, LV_PART_MAIN);
    text(humidity, "Humedad", 38, 8, 76, 18, kWhite);
    humidityValue_ = text(humidity, "--%", 38, 31, 76, 20, kWhite);

    auto* windPanel = lv_obj_create(card_);
    statPanels_[1] = windPanel;
    rect(windPanel, 300, 224, 124, 60);
    lv_obj_set_style_radius(windPanel, 14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(windPanel, LV_COLOR_MAKE(98, 183, 224), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(windPanel, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(windPanel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(windPanel, 0, LV_PART_MAIN);
    auto* windIcon = lv_image_create(windPanel);
    rect(windIcon, 10, 20, 22, 20);
    lv_image_set_src(windIcon, &wind);
    lv_image_set_scale(windIcon, 53);
    lv_obj_set_style_image_recolor(windIcon, kWhite, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(windIcon, LV_OPA_COVER, LV_PART_MAIN);
    text(windPanel, "Viento", 38, 8, 76, 18, kWhite);
    windValue_ = text(windPanel, "-- km/h", 38, 31, 80, 20, kWhite);

    for (size_t index = 0; index < dayCards_.size(); ++index) {
        auto* day = lv_obj_create(card_);
        rect(day, 16 + static_cast<int>(index) * 116, 292, 108, 100);
        lv_obj_clear_flag(day, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(day, 16, LV_PART_MAIN);
        lv_obj_set_style_border_width(day, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(day, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(day, LV_COLOR_MAKE(98, 183, 224), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(day, LV_OPA_60, LV_PART_MAIN);
        dayCards_[index] = day;
        dayLabels_[index] = text(day, "--", 4, 10, 100, 20, kWhite, LV_TEXT_ALIGN_CENTER);
        dayIcons_[index] = lv_image_create(day);
        // Source assets are 96 px.  Scale them explicitly; setting the object
        // size alone clips the image rather than resizing it in LVGL.
        rect(dayIcons_[index], 38, 33, 32, 32);
        lv_image_set_scale(dayIcons_[index], 85);
        lv_obj_set_style_image_recolor(dayIcons_[index], kWhite, LV_PART_MAIN);
        lv_obj_set_style_image_recolor_opa(dayIcons_[index], LV_OPA_COVER, LV_PART_MAIN);
        dayTemperatureLabels_[index] = text(day, "--° --°", 4, 75, 100, 18, kWhite, LV_TEXT_ALIGN_CENTER);
    }

    auto* footer = lv_obj_create(card_);
    rect(footer, 0, 408, 608, 36);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(footer, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(footer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(footer, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_border_color(footer, LV_COLOR_MAKE(180, 220, 238), LV_PART_MAIN);
    lv_obj_set_style_border_opa(footer, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_pad_all(footer, 0, LV_PART_MAIN);
    const auto controlChip = [&](int x, ControlIcon icon) {
        auto* chip = lv_obj_create(footer);
        rect(chip, x, 8, 22, 20);
        lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(chip, 4, LV_PART_MAIN);
        lv_obj_set_style_bg_color(chip, LV_COLOR_MAKE(120, 138, 156), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(chip, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_border_width(chip, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(chip, kWhite, LV_PART_MAIN);
        lv_obj_set_style_border_opa(chip, LV_OPA_40, LV_PART_MAIN);
        lv_obj_set_style_pad_all(chip, 0, LV_PART_MAIN);
        drawControlIcon(chip, icon);
    };
    const auto separator = [&](int x) {
        auto* line = lv_obj_create(footer);
        rect(line, x, 8, 1, 20);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(line, LV_COLOR_MAKE(124, 140, 156), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(line, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_border_width(line, 0, LV_PART_MAIN);
    };

    controlChip(117, ControlIcon::Up);
    controlChip(145, ControlIcon::Down);
    auto* cityControlLabel = text(footer, "Ciudad", 175, 8, 76, 20, LV_COLOR_MAKE(232, 239, 246));
    lv_obj_set_style_text_font(cityControlLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    separator(259);

    controlChip(281, ControlIcon::Left);
    controlChip(309, ControlIcon::Right);
    auto* dayControlLabel = text(footer, "Dia", 339, 8, 42, 20, LV_COLOR_MAKE(232, 239, 246));
    lv_obj_set_style_text_font(dayControlLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    separator(391);

    controlChip(415, ControlIcon::Confirm);
    auto* menuControlLabel = text(footer, "Menu", 445, 8, 64, 20, LV_COLOR_MAKE(232, 239, 246));
    lv_obj_set_style_text_font(menuControlLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    noticeLabel_ = text(card_, "", 70, 272, 470, 18, kWhite, LV_TEXT_ALIGN_CENTER);

    skeletonLayer_ = lv_obj_create(card_);
    rect(skeletonLayer_, 0, 0, 608, 392);
    lv_obj_clear_flag(skeletonLayer_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(skeletonLayer_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(skeletonLayer_, kBlue, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(skeletonLayer_, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(skeletonLayer_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(skeletonLayer_, 0, LV_PART_MAIN);
    const std::array<lv_area_t, 13> skeletonAreas{{
        {34, 24, 194, 46}, {34, 51, 118, 66}, {514, 32, 570, 38}, {40, 118, 140, 218},
        {164, 96, 403, 153}, {164, 168, 363, 188}, {164, 224, 287, 283}, {300, 224, 423, 283},
        {16, 292, 123, 391}, {132, 292, 239, 391}, {248, 292, 355, 391}, {364, 292, 471, 391},
        {480, 292, 587, 391},
    }};
    for (size_t index = 0; index < skeletonBlocks_.size(); ++index) {
        auto* block = lv_obj_create(skeletonLayer_);
        const auto& area = skeletonAreas[index];
        rect(block, area.x1, area.y1, area.x2 - area.x1 + 1, area.y2 - area.y1 + 1);
        lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(block, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(block, index < 2 ? 8 : 14, LV_PART_MAIN);
        lv_obj_set_style_bg_color(block, LV_COLOR_MAKE(220, 235, 245), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(block, LV_OPA_20, LV_PART_MAIN);
        lv_obj_set_style_border_width(block, 0, LV_PART_MAIN);
        skeletonBlocks_[index] = block;
    }
    setLoading(false);

    // A dimmed backdrop separates the city picker from the live weather view
    // without introducing a second full-screen colour band.
    modalBackdrop_ = lv_obj_create(root_);
    rect(modalBackdrop_, 0, 0, 640, 480);
    lv_obj_clear_flag(modalBackdrop_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(modalBackdrop_, LV_COLOR_MAKE(12, 18, 26), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(modalBackdrop_, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_width(modalBackdrop_, 0, LV_PART_MAIN);

    modal_ = lv_obj_create(root_);
    rect(modal_, 44, 86, 552, 284);
    lv_obj_clear_flag(modal_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(modal_, 18, LV_PART_MAIN);
    lv_obj_set_style_bg_color(modal_, LV_COLOR_MAKE(86, 96, 108), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(modal_, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(modal_, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(modal_, LV_COLOR_MAKE(188, 198, 208), LV_PART_MAIN);
    lv_obj_set_style_border_opa(modal_, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_pad_all(modal_, 0, LV_PART_MAIN);
    lv_obj_set_user_data(modal_, this);

    auto* modalHeader = lv_obj_create(modal_);
    rect(modalHeader, 0, 0, 552, 44);
    lv_obj_clear_flag(modalHeader, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(modalHeader, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(modalHeader, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(modalHeader, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(modalHeader, 0, LV_PART_MAIN);
    text(modalHeader, "ELEGIR CIUDAD", 16, 10, 250, 18, LV_COLOR_MAKE(229, 234, 240));
    constexpr std::array<int, 4> kModalWeatherCodes{{0, 2, 61, 0}};
    for (size_t index = 0; index < modalRows_.size(); ++index) {
        auto* row = lv_obj_create(modal_);
        rect(row, 0, 44 + static_cast<int>(index) * 55, 552, 55);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(row, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(row, LV_COLOR_MAKE(174, 183, 193), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(row, cityButtonEvent, LV_EVENT_CLICKED, nullptr);
        const auto& location = weatherLocations()[index];
        auto* cityIcon = lv_image_create(row);
        rect(cityIcon, 18, 16, 24, 24);
        lv_image_set_src(cityIcon, weatherImage(kModalWeatherCodes[index]));
        lv_image_set_scale(cityIcon, 64);
        lv_obj_set_style_image_recolor(cityIcon, kWhite, LV_PART_MAIN);
        lv_obj_set_style_image_recolor_opa(cityIcon, LV_OPA_COVER, LV_PART_MAIN);
        auto* cityName = text(row, location.name, 54, 9, 330, 20, kWhite);
        lv_obj_set_style_text_font(cityName, &lv_font_montserrat_16, LV_PART_MAIN);
        text(row, location.country, 54, 30, 330, 17, LV_COLOR_MAKE(220, 226, 233));
        text(row, "--", 466, 18, 56, 20, kWhite, LV_TEXT_ALIGN_RIGHT);
        modalRows_[index] = row;
    }
    // Rows are created after the header, so explicitly keep the title strip
    // above them; otherwise the selected row can cover the label by a pixel.
    lv_obj_move_foreground(modalHeader);
    setModalVisible(false);
    updateForecastSelection();
    rebuildWeatherIcon(3);
}

void WeatherScreen::selectCity(int index, bool fetch) {
    const int cityCount = static_cast<int>(weatherLocations().size());
    cityIndex_ = (index % cityCount + cityCount) % cityCount;
    dayIndex_ = 0;
    updateStaticData();
    updateForecastSelection();
    if (fetch) {
        // Keep the existing forecast visible on later refreshes. The skeleton
        // is only useful before the app has any weather information at all.
        if (!hasCurrentData_) setLoading(true);
        showNotice("Actualizando clima...");
        client_.fetch(weatherLocations()[static_cast<size_t>(cityIndex_)]);
    }
}

void WeatherScreen::updateStaticData() {
    const auto& location = weatherLocations()[static_cast<size_t>(cityIndex_)];
    lv_label_set_text(cityLabel_, location.name);
    lv_label_set_text(countryLabel_, location.country);
    updateCityIndicator(true);
}

void WeatherScreen::updateCityIndicator(bool animate) {
    int x = 514;
    for (size_t index = 0; index < cityIndicators_.size(); ++index) {
        auto* indicator = cityIndicators_[index];
        const bool active = static_cast<int>(index) == cityIndex_;
        const int width = active ? kActiveIndicatorWidth : kIndicatorWidth;
        const int opacity = active ? LV_OPA_COVER : LV_OPA_50;
        animateObjectValue(indicator, setObjectX, lv_obj_get_x(indicator), x, animate);
        animateObjectValue(indicator, setObjectWidth, lv_obj_get_width(indicator), width, animate);
        animateObjectValue(indicator, setObjectBackgroundOpacity,
                           lv_obj_get_style_bg_opa(indicator, LV_PART_MAIN), opacity, animate);
        x += width + kIndicatorGap;
    }
}

void WeatherScreen::setLoading(bool loading) {
    if (loading) lv_obj_clear_flag(skeletonLayer_, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(skeletonLayer_, LV_OBJ_FLAG_HIDDEN);

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

void WeatherScreen::update() {
    while (auto result = client_.takeResult()) {
        if (std::string(result->location.id) != weatherLocations()[static_cast<size_t>(cityIndex_)].id) continue;
        if (result->status == WeatherStatus::Ready) {
            applyWeather(*result);
        } else if (hasCurrentData_) {
            setLoading(false);
            showNotice("Sin conexion - mostrando ultimo dato");
        } else {
            setLoading(false);
            lv_label_set_text(temperatureLabel_, "-- C");
            lv_label_set_text(conditionLabel_, "Sin datos");
            lv_label_set_text(rangeLabel_, "No se pudieron cargar los datos");
            showNotice("Sin conexion");
        }
    }
}

void WeatherScreen::applyWeather(const WeatherData& data) {
    setLoading(false);
    currentData_ = data;
    hasCurrentData_ = true;
    char value[32];
    std::snprintf(value, sizeof(value), "%d°", data.temperature);
    lv_label_set_text(temperatureLabel_, value);
    lv_label_set_text(conditionLabel_, weatherDescription(data.weatherCode));
    std::snprintf(value, sizeof(value), "Max %d° / Min %d°", data.daily[0].maximum, data.daily[0].minimum);
    lv_label_set_text(rangeLabel_, value);
    std::snprintf(value, sizeof(value), "%d%%", data.humidity);
    lv_label_set_text(humidityValue_, value);
    std::snprintf(value, sizeof(value), "%d km/h", data.windSpeed);
    lv_label_set_text(windValue_, value);
    for (size_t index = 0; index < data.daily.size(); ++index) {
        lv_label_set_text(dayLabels_[index], shortDay(static_cast<int>(index)));
        lv_image_set_src(dayIcons_[index], weatherImage(data.daily[index].weatherCode));
        std::snprintf(value, sizeof(value), "%d° %d°", data.daily[index].maximum, data.daily[index].minimum);
        lv_label_set_text(dayTemperatureLabels_[index], value);
    }
    refreshTheme(data.weatherCode);
    rebuildWeatherIcon(data.weatherCode);
    showNotice("");
}

void WeatherScreen::rebuildWeatherIcon(int weatherCode) {
    lv_image_set_src(weatherVisual_, weatherImage(weatherCode));
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
    const lv_color_t dark = rainy ? kSlateDark : kBlueDark;
    lv_obj_set_style_bg_color(root_, dark, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(root_, base, LV_PART_MAIN);
    lv_obj_set_style_bg_color(card_, base, LV_PART_MAIN);
    lv_obj_set_style_bg_color(skeletonLayer_, base, LV_PART_MAIN);
    lv_color_t forecastColor = LV_COLOR_MAKE(98, 183, 224);
    if (rainy) forecastColor = LV_COLOR_MAKE(104, 121, 139);
    for (auto* day : dayCards_) {
        lv_obj_set_style_bg_color(day, forecastColor, LV_PART_MAIN);
    }
    for (auto* panel : statPanels_) {
        lv_obj_set_style_bg_color(panel, forecastColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(panel, LV_OPA_60, LV_PART_MAIN);
    }
}

void WeatherScreen::handleAction(int actionValue) {
    const auto action = static_cast<Action>(actionValue);
    if (modalVisible_) {
        if (action == Action::Up) { selectCity(cityIndex_ - 1, false); setModalVisible(true); }
        if (action == Action::Down) { selectCity(cityIndex_ + 1, false); setModalVisible(true); }
        if (action == Action::Confirm) { setModalVisible(false); selectCity(cityIndex_); }
        if (action == Action::Back) setModalVisible(false);
        return;
    }
    if (action == Action::Up) selectCity(cityIndex_ - 1);
    if (action == Action::Down) selectCity(cityIndex_ + 1);
    if (action == Action::Left) { dayIndex_ = (dayIndex_ + 4) % 5; updateForecastSelection(); }
    if (action == Action::Right) { dayIndex_ = (dayIndex_ + 1) % 5; updateForecastSelection(); }
    if (action == Action::Confirm) setModalVisible(true);
}

void WeatherScreen::setModalVisible(bool visible) {
    modalVisible_ = visible;
    if (visible) {
        lv_obj_clear_flag(modalBackdrop_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(modal_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(modalBackdrop_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(modal_, LV_OBJ_FLAG_HIDDEN);
    }
    for (size_t index = 0; index < 4; ++index) {
        const bool selected = static_cast<int>(index) == cityIndex_;
        lv_obj_set_style_bg_opa(modalRows_[index], selected ? LV_OPA_40 : LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

void WeatherScreen::updateForecastSelection() {
    for (size_t index = 0; index < dayCards_.size(); ++index) {
        lv_obj_set_style_border_width(dayCards_[index], static_cast<int>(index) == dayIndex_ ? 2 : 0, LV_PART_MAIN);
        lv_obj_set_style_border_color(dayCards_[index], kWhite, LV_PART_MAIN);
    }
}

void WeatherScreen::showNotice(const char* textValue) { lv_label_set_text(noticeLabel_, textValue); }

void WeatherScreen::cityButtonEvent(lv_event_t* event) {
    auto* row = static_cast<lv_obj_t*>(lv_event_get_target(event));
    auto* modal = lv_obj_get_parent(row);
    auto* screen = static_cast<WeatherScreen*>(lv_obj_get_user_data(modal));
    for (size_t index = 0; index < screen->modalRows_.size(); ++index) {
        if (screen->modalRows_[index] == row) {
            screen->cityIndex_ = static_cast<int>(index);
            screen->setModalVisible(false);
            screen->selectCity(static_cast<int>(index));
            return;
        }
    }
}
