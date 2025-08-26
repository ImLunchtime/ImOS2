#include "control_center.h"
#include "hals/hal_audio.h"
#include "hals/hal_display.h"
#include "lvgl.h"
#include <stdio.h>

// Declare the HarmonyOS Sans font
LV_FONT_DECLARE(yinpin_hm_light_20);

#define FLOATING_BAR_HEIGHT 50
#define FLOATING_BAR_WIDTH 600  // Increased from 500 to 600 to fit larger labels
#define FLOATING_BAR_MARGIN 20

typedef struct control_center_t {
    lv_obj_t *floating_bar;
    lv_obj_t *volume_slider;
    lv_obj_t *volume_label;
    lv_obj_t *volume_value_label;
    lv_obj_t *brightness_slider;
    lv_obj_t *brightness_label;
    lv_obj_t *brightness_value_label;
    bool is_initialized;
} control_center_t;

static control_center_t g_control_center = {0};

static void volume_slider_event(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    hal_set_speaker_volume((uint8_t)value);
}

static void volume_label_update_event(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *slider = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t*)lv_event_get_user_data(e);
    int32_t value = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d%%", (int)value);
}

static void brightness_slider_event(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    hal_display_set_brightness((uint8_t)value);
}

static void brightness_label_update_event(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *slider = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t*)lv_event_get_user_data(e);
    int32_t value = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d%%", (int)value);
}

static void create_floating_bar_ui(void)
{
    // Get screen dimensions
    lv_coord_t screen_width = lv_display_get_horizontal_resolution(NULL);
    lv_coord_t screen_height = lv_display_get_vertical_resolution(NULL);
    
    // Create the floating bar container with Liquid Glass style
    g_control_center.floating_bar = lv_obj_create(lv_screen_active());
    lv_obj_set_size(g_control_center.floating_bar, FLOATING_BAR_WIDTH, FLOATING_BAR_HEIGHT);
    
    // Position at bottom center with margin
    lv_obj_align(g_control_center.floating_bar, LV_ALIGN_BOTTOM_MID, 0, -FLOATING_BAR_MARGIN);
    
    // Apply Liquid Glass styling (pill-shaped)
    lv_obj_set_style_radius(g_control_center.floating_bar, FLOATING_BAR_HEIGHT / 2, 0); // Pill shape
    lv_obj_set_style_bg_color(g_control_center.floating_bar, lv_color_hex(0xf6f6f6), 0);
    lv_obj_set_style_bg_opa(g_control_center.floating_bar, LV_OPA_90, 0); // Slightly transparent
    lv_obj_set_style_border_width(g_control_center.floating_bar, 1, 0);
    lv_obj_set_style_border_color(g_control_center.floating_bar, lv_color_white(), 0);
    
    // Add shadow effect for Liquid Glass look
    lv_obj_set_style_shadow_width(g_control_center.floating_bar, 15, 0);
    lv_obj_set_style_shadow_spread(g_control_center.floating_bar, 2, 0);
    lv_obj_set_style_shadow_ofs_x(g_control_center.floating_bar, 0, 0);
    lv_obj_set_style_shadow_ofs_y(g_control_center.floating_bar, 3, 0);
    lv_obj_set_style_shadow_opa(g_control_center.floating_bar, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(g_control_center.floating_bar, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
    
    // Compact padding for pill shape
    lv_obj_set_style_pad_all(g_control_center.floating_bar, 8, 0);
    
    // Make the bar clickable and disable scrolling
    lv_obj_clear_flag(g_control_center.floating_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_control_center.floating_bar, LV_OBJ_FLAG_CLICKABLE);
    
    // Keep the bar always on top
    lv_obj_move_foreground(g_control_center.floating_bar);
    
    // Create horizontal container for all controls
    lv_obj_t *controls_container = lv_obj_create(g_control_center.floating_bar);
    lv_obj_set_size(controls_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(controls_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(controls_container, 0, 0);
    lv_obj_set_style_pad_all(controls_container, 0, 0);
    lv_obj_set_flex_flow(controls_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Disable scrolling in the container as well
    lv_obj_clear_flag(controls_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(controls_container, LV_OBJ_FLAG_CLICKABLE);
    
    // === VOLUME CONTROLS ===
    // Volume icon/label
    g_control_center.volume_label = lv_label_create(controls_container);
    lv_label_set_text(g_control_center.volume_label, "音量");
    lv_obj_set_style_text_color(g_control_center.volume_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_control_center.volume_label, &yinpin_hm_light_20, 0);
    lv_obj_set_style_margin_right(g_control_center.volume_label, 8, 0);
    lv_obj_set_width(g_control_center.volume_label, 40);
    lv_label_set_long_mode(g_control_center.volume_label, LV_LABEL_LONG_CLIP);
    
    // Volume slider
    g_control_center.volume_slider = lv_slider_create(controls_container);
    lv_obj_set_width(g_control_center.volume_slider, 120); // Reduced to fit both sliders
    lv_obj_set_height(g_control_center.volume_slider, 16);
    lv_slider_set_range(g_control_center.volume_slider, 0, 100);
    
    // Set initial value from current audio HAL volume
    uint8_t current_volume = hal_get_speaker_volume();
    lv_slider_set_value(g_control_center.volume_slider, current_volume, LV_ANIM_OFF);
    
    // Style the volume slider
    lv_obj_set_style_bg_color(g_control_center.volume_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_control_center.volume_slider, lv_color_hex(0x00A8FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_control_center.volume_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_radius(g_control_center.volume_slider, LV_RADIUS_CIRCLE, 0);
    
    // Add volume slider events
    lv_obj_add_flag(g_control_center.volume_slider, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_control_center.volume_slider, volume_slider_event, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Volume value label
    g_control_center.volume_value_label = lv_label_create(controls_container);
    lv_label_set_text_fmt(g_control_center.volume_value_label, "%d%%", current_volume);
    lv_obj_set_style_text_color(g_control_center.volume_value_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_control_center.volume_value_label, &yinpin_hm_light_20, 0);
    lv_obj_set_style_margin_left(g_control_center.volume_value_label, 8, 0);
    lv_obj_set_style_margin_right(g_control_center.volume_value_label, 15, 0); // Space between controls
    lv_obj_set_width(g_control_center.volume_value_label, 35);
    lv_label_set_long_mode(g_control_center.volume_value_label, LV_LABEL_LONG_CLIP);
    
    // Update volume value label when slider changes
    lv_obj_add_event_cb(g_control_center.volume_slider, volume_label_update_event, LV_EVENT_VALUE_CHANGED, g_control_center.volume_value_label);
    
    // === BRIGHTNESS CONTROLS ===
    // Brightness icon/label
    g_control_center.brightness_label = lv_label_create(controls_container);
    lv_label_set_text(g_control_center.brightness_label, "亮度");
    lv_obj_set_style_text_color(g_control_center.brightness_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_control_center.brightness_label, &yinpin_hm_light_20, 0);
    lv_obj_set_style_margin_right(g_control_center.brightness_label, 8, 0);
    lv_obj_set_width(g_control_center.brightness_label, 40);
    lv_label_set_long_mode(g_control_center.brightness_label, LV_LABEL_LONG_CLIP);
    
    // Brightness slider
    g_control_center.brightness_slider = lv_slider_create(controls_container);
    lv_obj_set_width(g_control_center.brightness_slider, 120); // Same width as volume slider
    lv_obj_set_height(g_control_center.brightness_slider, 16);
    lv_slider_set_range(g_control_center.brightness_slider, 0, 100);
    
    // Set initial value from current display brightness
    uint8_t current_brightness = hal_display_get_brightness();
    lv_slider_set_value(g_control_center.brightness_slider, current_brightness, LV_ANIM_OFF);
    
    // Style the brightness slider with different color
    lv_obj_set_style_bg_color(g_control_center.brightness_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_control_center.brightness_slider, lv_color_hex(0xFFB000), LV_PART_INDICATOR); // Orange for brightness
    lv_obj_set_style_bg_color(g_control_center.brightness_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_radius(g_control_center.brightness_slider, LV_RADIUS_CIRCLE, 0);
    
    // Add brightness slider events
    lv_obj_add_flag(g_control_center.brightness_slider, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_control_center.brightness_slider, brightness_slider_event, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Brightness value label
    g_control_center.brightness_value_label = lv_label_create(controls_container);
    lv_label_set_text_fmt(g_control_center.brightness_value_label, "%d%%", current_brightness);
    lv_obj_set_style_text_color(g_control_center.brightness_value_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_control_center.brightness_value_label, &yinpin_hm_light_20, 0);
    lv_obj_set_style_margin_left(g_control_center.brightness_value_label, 8, 0);
    lv_obj_set_width(g_control_center.brightness_value_label, 35);
    lv_label_set_long_mode(g_control_center.brightness_value_label, LV_LABEL_LONG_CLIP);
    
    // Update brightness value label when slider changes
    lv_obj_add_event_cb(g_control_center.brightness_slider, brightness_label_update_event, LV_EVENT_VALUE_CHANGED, g_control_center.brightness_value_label);
    
    printf("Floating control bar created at bottom center with size (%d x %d)\n", 
           FLOATING_BAR_WIDTH, FLOATING_BAR_HEIGHT);
}

void control_center_init(void)
{
    if (g_control_center.is_initialized) {
        return; // Already initialized
    }
    
    create_floating_bar_ui();
    g_control_center.is_initialized = true;
    
    printf("Control center floating bar initialized and clickable\n");
}

void control_center_show(void)
{
    if (g_control_center.floating_bar) {
        lv_obj_clear_flag(g_control_center.floating_bar, LV_OBJ_FLAG_HIDDEN);
    }
}

void control_center_hide(void)
{
    if (g_control_center.floating_bar) {
        lv_obj_add_flag(g_control_center.floating_bar, LV_OBJ_FLAG_HIDDEN);
    }
}

bool control_center_is_visible(void)
{
    if (!g_control_center.is_initialized || !g_control_center.floating_bar) {
        return false;
    }
    return !lv_obj_has_flag(g_control_center.floating_bar, LV_OBJ_FLAG_HIDDEN);
}

void control_center_deinit(void)
{
    if (!g_control_center.is_initialized) {
        return;
    }
    
    if (g_control_center.floating_bar) {
        lv_obj_del(g_control_center.floating_bar);
        g_control_center.floating_bar = NULL;
        g_control_center.volume_slider = NULL;
        g_control_center.volume_label = NULL;
        g_control_center.volume_value_label = NULL;
        g_control_center.brightness_slider = NULL;
        g_control_center.brightness_label = NULL;
        g_control_center.brightness_value_label = NULL;
    }
    
    g_control_center.is_initialized = false;
}

// Return 0 since the floating bar doesn't affect layout like the old docked bar
lv_coord_t control_center_get_bar_height(void)
{
    return 0; // Floating bar doesn't reserve screen space
}