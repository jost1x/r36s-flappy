/* Project-level LVGL configuration. Unspecified options use LVGL defaults. */
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 32
/* The dashboard uses multiple A8 masks, animated skeletons and modal layers. */
#define LV_MEM_SIZE (512 * 1024U)
/* Weather icons are generated as LV_COLOR_FORMAT_A8 alpha masks. */
#define LV_DRAW_SW_SUPPORT_A8 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_48 1

#endif
