#include "theme_engine.h"
#include "lvgl.h"

void theme_apply_button_style(lv_obj_t *btn)
{
    if (!btn) return;
    
    // Set button size (same as launcher buttons)
    lv_obj_set_size(btn, 110, 55);
    
    // Apply pill-shaped styling
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    
    // White background
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
    
    // No border
    lv_obj_set_style_border_width(btn, 0, 0);
    
    // No shadow (as per launcher buttons)
    lv_obj_set_style_shadow_width(btn, 0, 0);
    
    // Standard padding
    lv_obj_set_style_pad_all(btn, 8, 0);
    
    // Ensure button is clickable
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
}

void theme_apply_label_style(lv_obj_t *label)
{
    if (!label) return;
    
    // Apply custom font
    lv_obj_set_style_text_font(label, &yinpin_hm_light_20, 0);
    
    // Default text color (black)
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    
    // Left align text by default (changed from center)
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
}

void theme_apply_panel_style(lv_obj_t *panel)
{
    if (!panel) return;
    
    // Apply liquid glass styling (same as window manager)
    lv_obj_set_style_radius(panel, 20, 0);
    lv_obj_set_style_pad_all(panel, 12, 0);
    
    // Default background color
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xf6f6f6), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_100, 0);
    
    // White border
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_white(), 0);
    
    // Shadow effects
    lv_obj_set_style_shadow_width(panel, 20, 0);
    lv_obj_set_style_shadow_spread(panel, 2, 0);
    lv_obj_set_style_shadow_ofs_x(panel, 0, 0);
    lv_obj_set_style_shadow_ofs_y(panel, 5, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(panel, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
}

void theme_apply_panel_style_with_color(lv_obj_t *panel, lv_color_t bg_color)
{
    if (!panel) return;
    
    // Apply base panel style first
    theme_apply_panel_style(panel);
    
    // Override background color
    lv_obj_set_style_bg_color(panel, bg_color, 0);
}

void theme_apply_button_icon_style(lv_obj_t *icon_label)
{
    if (!icon_label) return;
    
    // Set icon color to black for visibility on white buttons
    lv_obj_set_style_text_color(icon_label, lv_color_black(), 0);
    
    // Center the icon
    lv_obj_center(icon_label);
}

void theme_engine_init(void)
{
    // Theme engine initialization
    // Currently no specific initialization needed
    // This function is reserved for future theme configuration
}