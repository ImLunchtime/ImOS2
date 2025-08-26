#pragma once

#include "lvgl.h"

/**
 * Theme Engine for Liquid Glass Style
 * Provides consistent styling functions for LVGL objects
 */

// Font declarations
LV_FONT_DECLARE(yinpin_hm_light_20);

/**
 * Apply liquid glass button style (pill-shaped white buttons)
 * @param btn Button object to style
 */
void theme_apply_button_style(lv_obj_t *btn);

/**
 * Apply standard label style with custom font
 * @param label Label object to style
 */
void theme_apply_label_style(lv_obj_t *label);

/**
 * Apply liquid glass panel style (same as window manager panels)
 * @param panel Panel object to style
 */
void theme_apply_panel_style(lv_obj_t *panel);

/**
 * Apply liquid glass panel style with custom background color
 * @param panel Panel object to style
 * @param bg_color Custom background color
 */
void theme_apply_panel_style_with_color(lv_obj_t *panel, lv_color_t bg_color);

/**
 * Initialize the theme engine
 */
void theme_engine_init(void);

void theme_apply_button_icon_style(lv_obj_t *icon_label);