#include "weather/weather_icons.h"

#include <cassert>
#include <cstdint>
#include <vector>

void runIconRenderTest() {
    lv_init();
    std::vector<uint32_t> buffer(96U * 96U, 0U);
    lv_display_t* display = lv_display_create(96, 96);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_ARGB8888);
    lv_display_set_buffers(display, buffer.data(), nullptr, buffer.size() * sizeof(uint32_t), LV_DISPLAY_RENDER_MODE_FULL);

    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, LV_COLOR_MAKE(0, 0, 0), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_t* image = lv_image_create(screen);
    lv_image_set_src(image, &cloud);
    lv_obj_set_style_image_recolor(image, LV_COLOR_MAKE(255, 255, 255), LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(image, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_opa(image, LV_OPA_COVER, LV_PART_MAIN);
    lv_scr_load(screen);
    lv_refr_now(display);

    bool hasIconPixel = false;
    for (const auto pixel : buffer) {
        if (pixel != 0U && pixel != 0xFF000000U) { hasIconPixel = true; break; }
    }
    assert(hasIconPixel);
    lv_display_delete(display);
    lv_deinit();
}
