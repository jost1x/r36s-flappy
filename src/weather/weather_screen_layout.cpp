#include "weather/weather_screen.h"

#include "weather/app_fonts.h"
#include "weather/weather_icons.h"

#include <array>

namespace {
constexpr lv_color_t kWhite = LV_COLOR_MAKE(245, 248, 252);
constexpr lv_color_t kBlue = LV_COLOR_MAKE(40, 157, 221);
constexpr lv_color_t kBlueDark = LV_COLOR_MAKE(40, 131, 183);
constexpr int kIndicatorHeight = 6;
constexpr int kIndicatorWidth = 6;
constexpr int kIndicatorY = 35;

enum class ControlIcon { Left, Right, Up, Down, Confirm };
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

void drawLine(lv_obj_t* parent, const lv_point_precise_t points[], size_t count) {
    auto* line = lv_line_create(parent);
    // Keep the line's coordinate system equal to its 22 x 20 control chip so
    // the arrow paths remain centered regardless of LVGL's default line size.
    rect(line, 0, 0, 22, 20);
    lv_line_set_points(line, points, count);
    lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(line, kWhite, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN);
}

void drawControlIcon(lv_obj_t* parent, ControlIcon icon) {
    switch (icon) {
        case ControlIcon::Left: drawLine(parent, kLeftHead, 3); drawLine(parent, kLeftShaft, 2); break;
        case ControlIcon::Right: drawLine(parent, kRightHead, 3); drawLine(parent, kRightShaft, 2); break;
        case ControlIcon::Up: drawLine(parent, kUpHead, 3); drawLine(parent, kUpShaft, 2); break;
        case ControlIcon::Down: drawLine(parent, kDownHead, 3); drawLine(parent, kDownShaft, 2); break;
        case ControlIcon::Confirm: drawLine(parent, kConfirmBody, 3); drawLine(parent, kConfirmBar, 2); break;
    }
}

void transparentContainer(lv_obj_t* object) {
    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(object, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(object, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(object, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(object, 0, LV_PART_MAIN);
}

void translucentPanel(lv_obj_t* panel) {
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(panel, 14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, LV_COLOR_MAKE(98, 183, 224), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
}
}  // namespace

void WeatherScreen::build() {
    root_ = lv_obj_create(nullptr);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root_, kBlueDark, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(root_, kBlue, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(root_, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(root_, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(root_, &r36s_font_14, LV_PART_MAIN);

    card_ = lv_obj_create(root_);
    rect(card_, 16, 16, 608, 448);
    transparentContainer(card_);
    cityLabel_ = text(card_, "Santiago", 34, 24, 220, 24, kWhite);
    countryLabel_ = text(card_, "Chile", 34, 45, 220, 20, LV_COLOR_MAKE(210, 238, 255));
    lv_obj_set_style_text_font(cityLabel_, &r36s_font_20, LV_PART_MAIN);
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

    weatherVisual_ = lv_image_create(card_);
    rect(weatherVisual_, 40, 118, 100, 100);
    lv_image_set_src(weatherVisual_, &cloud);
    lv_obj_set_style_image_opa(weatherVisual_, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(weatherVisual_, kWhite, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(weatherVisual_, LV_OPA_COVER, LV_PART_MAIN);
    conditionIcon_ = text(card_, "", 0, 0, 0, 0, kWhite);
    lv_obj_add_flag(conditionIcon_, LV_OBJ_FLAG_HIDDEN);
    temperatureLabel_ = text(card_, "--°", 164, 96, 240, 62, kWhite);
    conditionLabel_ = text(card_, "Cargando...", 164, 168, 280, 28, kWhite);
    rangeLabel_ = text(card_, "Max --° / Min --°", 164, 196, 280, 24, LV_COLOR_MAKE(220, 240, 250));
    lv_obj_set_style_text_font(temperatureLabel_, &r36s_font_48, LV_PART_MAIN);
    lv_obj_set_style_text_font(conditionLabel_, &r36s_font_20, LV_PART_MAIN);
    text(card_, "Hoy", 70, 218, 75, 20, kWhite, LV_TEXT_ALIGN_CENTER);

    const auto statPanel = [&](size_t index, int x, int width, const char* title, lv_obj_t** value) {
        auto* panel = lv_obj_create(card_);
        statPanels_[index] = panel;
        rect(panel, x, 224, width, 60);
        translucentPanel(panel);
        text(panel, title, 4, 8, width - 8, 18, kWhite, LV_TEXT_ALIGN_CENTER);
        *value = text(panel, index == 1 ? "-- km/h" : index == 0 || index == 3 ? "--%" : "--°", 4, 31, width - 8, 20, kWhite, LV_TEXT_ALIGN_CENTER);
        return panel;
    };
    auto* humidity = statPanel(0, 164, 124, "Humedad", &humidityValue_);
    auto* humidityIcon = lv_image_create(humidity); rect(humidityIcon, 11, 20, 20, 20); lv_image_set_src(humidityIcon, &droplets); lv_image_set_scale(humidityIcon, 53);
    lv_obj_set_style_image_recolor(humidityIcon, kWhite, LV_PART_MAIN); lv_obj_set_style_image_recolor_opa(humidityIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_x(humidityValue_, 38); lv_obj_set_width(humidityValue_, 76); lv_obj_set_style_text_align(humidityValue_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_x(lv_obj_get_child(humidity, 0), 38); lv_obj_set_width(lv_obj_get_child(humidity, 0), 76); lv_obj_set_style_text_align(lv_obj_get_child(humidity, 0), LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    auto* windPanel = statPanel(1, 300, 124, "Viento", &windValue_);
    auto* windIcon = lv_image_create(windPanel); rect(windIcon, 10, 20, 22, 20); lv_image_set_src(windIcon, &wind); lv_image_set_scale(windIcon, 53);
    lv_obj_set_style_image_recolor(windIcon, kWhite, LV_PART_MAIN); lv_obj_set_style_image_recolor_opa(windIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_x(windValue_, 38); lv_obj_set_width(windValue_, 80); lv_obj_set_style_text_align(windValue_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_x(lv_obj_get_child(windPanel, 0), 38); lv_obj_set_width(lv_obj_get_child(windPanel, 0), 76); lv_obj_set_style_text_align(lv_obj_get_child(windPanel, 0), LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    statPanel(2, 436, 76, "Sens.", &feelsValue_);
    statPanel(3, 524, 68, "Lluvia", &rainValue_);

    for (size_t index = 0; index < dayCards_.size(); ++index) {
        auto* day = lv_obj_create(card_);
        rect(day, 16 + static_cast<int>(index) * 116, 292, 108, 100);
        translucentPanel(day);
        lv_obj_set_style_radius(day, 16, LV_PART_MAIN);
        dayCards_[index] = day;
        dayLabels_[index] = text(day, "--", 4, 10, 100, 20, kWhite, LV_TEXT_ALIGN_CENTER);
        dayIcons_[index] = lv_image_create(day);
        rect(dayIcons_[index], 38, 33, 32, 32);
        lv_image_set_src(dayIcons_[index], &cloud);
        lv_image_set_scale(dayIcons_[index], 85);
        lv_obj_set_style_image_opa(dayIcons_[index], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_image_recolor(dayIcons_[index], kWhite, LV_PART_MAIN);
        lv_obj_set_style_image_recolor_opa(dayIcons_[index], LV_OPA_COVER, LV_PART_MAIN);
        dayTemperatureLabels_[index] = text(day, "--° --°", 4, 75, 100, 18, kWhite, LV_TEXT_ALIGN_CENTER);
    }

    auto* footer = lv_obj_create(card_);
    rect(footer, 0, 408, 608, 36);
    transparentContainer(footer);
    lv_obj_set_style_border_width(footer, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_border_color(footer, LV_COLOR_MAKE(180, 220, 238), LV_PART_MAIN);
    lv_obj_set_style_border_opa(footer, LV_OPA_30, LV_PART_MAIN);
    const auto controlChip = [&](int x, ControlIcon icon) {
        auto* chip = lv_obj_create(footer); rect(chip, x, 8, 22, 20); translucentPanel(chip);
        lv_obj_set_style_radius(chip, 4, LV_PART_MAIN); lv_obj_set_style_bg_color(chip, LV_COLOR_MAKE(120, 138, 156), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(chip, LV_OPA_50, LV_PART_MAIN); lv_obj_set_style_border_width(chip, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(chip, kWhite, LV_PART_MAIN); lv_obj_set_style_border_opa(chip, LV_OPA_40, LV_PART_MAIN); drawControlIcon(chip, icon);
    };
    const auto separator = [&](int x) { auto* line = lv_obj_create(footer); rect(line, x, 8, 1, 20); transparentContainer(line); lv_obj_set_style_bg_color(line, LV_COLOR_MAKE(124, 140, 156), LV_PART_MAIN); lv_obj_set_style_bg_opa(line, LV_OPA_30, LV_PART_MAIN); };
    controlChip(117, ControlIcon::Up); controlChip(145, ControlIcon::Down); text(footer, "Ciudad", 175, 8, 76, 20, LV_COLOR_MAKE(232, 239, 246)); separator(259);
    controlChip(281, ControlIcon::Left); controlChip(309, ControlIcon::Right); text(footer, "Día", 339, 8, 42, 20, LV_COLOR_MAKE(232, 239, 246)); separator(391);
    controlChip(415, ControlIcon::Confirm); text(footer, "Detalle", 445, 8, 74, 20, LV_COLOR_MAKE(232, 239, 246));
    noticeLabel_ = text(card_, "", 70, 272, 470, 18, kWhite, LV_TEXT_ALIGN_CENTER);
    updatedLabel_ = text(card_, "", 404, 48, 175, 18, LV_COLOR_MAKE(220, 240, 250), LV_TEXT_ALIGN_RIGHT);

    skeletonLayer_ = lv_obj_create(card_); rect(skeletonLayer_, 0, 0, 608, 392); transparentContainer(skeletonLayer_);
    lv_obj_remove_flag(skeletonLayer_, LV_OBJ_FLAG_CLICKABLE); lv_obj_set_style_bg_color(skeletonLayer_, kBlue, LV_PART_MAIN); lv_obj_set_style_bg_opa(skeletonLayer_, LV_OPA_COVER, LV_PART_MAIN);
    const std::array<lv_area_t, 13> skeletonAreas{{{34,24,194,46},{34,51,118,66},{514,32,570,38},{40,118,140,218},{164,96,403,153},{164,168,363,188},{164,224,287,283},{300,224,423,283},{16,292,123,391},{132,292,239,391},{248,292,355,391},{364,292,471,391},{480,292,587,391}}};
    for (size_t index = 0; index < skeletonBlocks_.size(); ++index) { auto* block = lv_obj_create(skeletonLayer_); const auto& area = skeletonAreas[index]; rect(block, area.x1, area.y1, area.x2 - area.x1 + 1, area.y2 - area.y1 + 1); transparentContainer(block); lv_obj_remove_flag(block, LV_OBJ_FLAG_CLICKABLE); lv_obj_set_style_radius(block, index < 2 ? 8 : 14, LV_PART_MAIN); lv_obj_set_style_bg_color(block, LV_COLOR_MAKE(220,235,245), LV_PART_MAIN); lv_obj_set_style_bg_opa(block, LV_OPA_20, LV_PART_MAIN); skeletonBlocks_[index] = block; }
    setLoading(false);

    modalBackdrop_ = lv_obj_create(root_); rect(modalBackdrop_, 0, 0, 640, 480); transparentContainer(modalBackdrop_); lv_obj_set_style_bg_color(modalBackdrop_, LV_COLOR_MAKE(12,18,26), LV_PART_MAIN); lv_obj_set_style_bg_opa(modalBackdrop_, LV_OPA_70, LV_PART_MAIN);
    modal_ = lv_obj_create(root_); rect(modal_, 44, 86, 552, 284); translucentPanel(modal_); lv_obj_set_style_radius(modal_, 18, LV_PART_MAIN); lv_obj_set_style_bg_color(modal_, LV_COLOR_MAKE(86,96,108), LV_PART_MAIN); lv_obj_set_style_bg_opa(modal_, LV_OPA_COVER, LV_PART_MAIN); lv_obj_set_style_border_width(modal_, 1, LV_PART_MAIN); lv_obj_set_style_border_color(modal_, LV_COLOR_MAKE(188,198,208), LV_PART_MAIN); lv_obj_set_style_border_opa(modal_, LV_OPA_40, LV_PART_MAIN); lv_obj_set_user_data(modal_, this);
    auto* header = lv_obj_create(modal_); rect(header, 0, 0, 552, 44); transparentContainer(header);
    modalTitle_ = text(header, "FAVORITOS  [DER: CATÁLOGO]", 16, 10, 360, 18, LV_COLOR_MAKE(229,234,240));
    modalPageLabel_ = text(header, "", 390, 10, 145, 18, LV_COLOR_MAKE(220,226,233), LV_TEXT_ALIGN_RIGHT);
    constexpr std::array<int, 4> codes{{0, 2, 61, 0}};
    for (size_t index = 0; index < modalRows_.size(); ++index) { auto* row = lv_obj_create(modal_); rect(row, 0, 44 + static_cast<int>(index) * 55, 552, 55); transparentContainer(row); lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE); lv_obj_set_style_bg_color(row, LV_COLOR_MAKE(174,183,193), LV_PART_MAIN); lv_obj_add_event_cb(row, cityButtonEvent, LV_EVENT_CLICKED, nullptr); auto* icon = lv_image_create(row); rect(icon, 18, 16, 24, 24); lv_image_set_src(icon, weatherImage(codes[index])); lv_image_set_scale(icon, 64); lv_obj_set_style_image_recolor(icon, kWhite, LV_PART_MAIN); lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, LV_PART_MAIN); modalRowNames_[index] = text(row, "", 54, 9, 330, 20, kWhite); lv_obj_set_style_text_font(modalRowNames_[index], &r36s_font_16, LV_PART_MAIN); modalRowDetails_[index] = text(row, "", 54, 30, 430, 17, LV_COLOR_MAKE(220,226,233)); modalRows_[index] = row; }
    lv_obj_move_foreground(header);
    setModalVisible(false);
    updateModalRows();
    updateForecastSelection();
    rebuildWeatherIcon(3);

    detail_ = lv_obj_create(root_);
    rect(detail_, 0, 0, 640, 480);
    transparentContainer(detail_);
    lv_obj_set_style_bg_color(detail_, kBlueDark, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(detail_, kBlue, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(detail_, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(detail_, LV_OPA_COVER, LV_PART_MAIN);

    auto* detailCard = lv_obj_create(detail_);
    rect(detailCard, 16, 16, 608, 448);
    transparentContainer(detailCard);
    detailCityLabel_ = text(detailCard, "Santiago", 30, 24, 320, 28, kWhite);
    lv_obj_set_style_text_font(detailCityLabel_, &r36s_font_20, LV_PART_MAIN);
    detailDayLabel_ = text(detailCard, "Hoy", 30, 54, 420, 22, LV_COLOR_MAKE(210, 238, 255));
    detailIcon_ = lv_image_create(detailCard);
    rect(detailIcon_, 50, 118, 132, 132);
    lv_image_set_src(detailIcon_, &cloud);
    lv_obj_set_style_image_opa(detailIcon_, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(detailIcon_, kWhite, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(detailIcon_, LV_OPA_COVER, LV_PART_MAIN);
    detailConditionLabel_ = text(detailCard, "--", 210, 126, 340, 32, kWhite);
    lv_obj_set_style_text_font(detailConditionLabel_, &r36s_font_20, LV_PART_MAIN);
    detailRangeLabel_ = text(detailCard, "Máxima --° / Mínima --°", 210, 172, 340, 28, LV_COLOR_MAKE(220, 240, 250));
    auto detailMetricIcon = [&](const lv_image_dsc_t* source, int y) {
        auto* icon = lv_image_create(detailCard);
        rect(icon, 178, y, 20, 20);
        lv_image_set_src(icon, source);
        lv_image_set_scale(icon, 53);
        lv_obj_set_style_image_recolor(icon, kWhite, LV_PART_MAIN);
        lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, LV_PART_MAIN);
    };
    detailMetricIcon(&droplets, 218);
    detailMetricIcon(&cloud_rain, 250);
    detailRainLabel_ = text(detailCard, "Probabilidad de lluvia: --%", 210, 214, 340, 26, kWhite);
    detailPrecipitationLabel_ = text(detailCard, "Precipitación esperada: -- mm", 210, 246, 340, 26, kWhite);

    auto* hint = lv_obj_create(detailCard);
    rect(hint, 0, 408, 608, 36);
    transparentContainer(hint);
    lv_obj_set_style_border_width(hint, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(hint, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_border_color(hint, LV_COLOR_MAKE(180, 220, 238), LV_PART_MAIN);
    lv_obj_set_style_border_opa(hint, LV_OPA_30, LV_PART_MAIN);
    const auto detailControlChip = [&](int x, ControlIcon icon) {
        auto* chip = lv_obj_create(hint);
        rect(chip, x, 8, 22, 20);
        translucentPanel(chip);
        lv_obj_set_style_radius(chip, 4, LV_PART_MAIN);
        lv_obj_set_style_bg_color(chip, LV_COLOR_MAKE(120, 138, 156), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(chip, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_border_width(chip, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(chip, kWhite, LV_PART_MAIN);
        lv_obj_set_style_border_opa(chip, LV_OPA_40, LV_PART_MAIN);
        drawControlIcon(chip, icon);
    };
    detailControlChip(112, ControlIcon::Left);
    detailControlChip(140, ControlIcon::Right);
    text(hint, "Cambiar día", 170, 8, 110, 20, LV_COLOR_MAKE(232, 239, 246));
    const auto detailLetterChip = [&](int x, const char* value) {
        auto* chip = lv_obj_create(hint);
        rect(chip, x, 8, 22, 20);
        translucentPanel(chip);
        lv_obj_set_style_radius(chip, 4, LV_PART_MAIN);
        lv_obj_set_style_bg_color(chip, LV_COLOR_MAKE(120, 138, 156), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(chip, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_border_width(chip, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(chip, kWhite, LV_PART_MAIN);
        lv_obj_set_style_border_opa(chip, LV_OPA_40, LV_PART_MAIN);
        text(chip, value, 0, 0, 22, 20, kWhite, LV_TEXT_ALIGN_CENTER);
    };
    detailLetterChip(330, "A");
    detailLetterChip(358, "B");
    text(hint, "Volver", 388, 8, 76, 20, LV_COLOR_MAKE(232, 239, 246));
    setDetailVisible(false);
}
