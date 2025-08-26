#include "control_center.h"
#include "hals/hal_audio.h"
#include "lvgl.h"
#include <stdio.h>

// Declare the HarmonyOS Sans font
LV_FONT_DECLARE(yinpin_hm_20);

#define BOTTOM_BAR_HEIGHT 60

typedef struct control_center_t {
    lv_obj_t *bottom_bar;
    lv_obj_t *volume_slider;
    lv_obj_t *volume_label;
    lv_obj_t *value_label;
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

static void create_bottom_bar_ui(void)
{
    // Get screen dimensions
    lv_coord_t screen_width = lv_display_get_horizontal_resolution(NULL);
    lv_coord_t screen_height = lv_display_get_vertical_resolution(NULL);
    
    // Create the bottom bar container
    g_control_center.bottom_bar = lv_obj_create(lv_screen_active());
    lv_obj_set_size(g_control_center.bottom_bar, screen_width, BOTTOM_BAR_HEIGHT);
    lv_obj_set_pos(g_control_center.bottom_bar, 0, screen_height - BOTTOM_BAR_HEIGHT);
    
    // Style the bottom bar with transparent-white gradient
    lv_obj_set_style_bg_color(g_control_center.bottom_bar, lv_color_white(), 0);
    lv_obj_set_style_bg_grad_color(g_control_center.bottom_bar, lv_color_white(), 0);
    lv_obj_set_style_bg_grad_dir(g_control_center.bottom_bar, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(g_control_center.bottom_bar, 0, 0);     // Start at 0% with full opacity
    lv_obj_set_style_bg_grad_stop(g_control_center.bottom_bar, 255, 0);   // End at 100% with full opacity
    lv_obj_set_style_bg_opa(g_control_center.bottom_bar, LV_OPA_30, 0);   // Overall transparency (30% opacity)
    lv_obj_set_style_border_width(g_control_center.bottom_bar, 1, 0);
    lv_obj_set_style_border_color(g_control_center.bottom_bar, lv_color_hex(0xE0E0E0), 0); // Light gray border
    lv_obj_set_style_border_side(g_control_center.bottom_bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_radius(g_control_center.bottom_bar, 0, 0);
    lv_obj_set_style_pad_all(g_control_center.bottom_bar, 10, 0);
    
    // Make the bar clickable and ensure it can receive touch events
    lv_obj_clear_flag(g_control_center.bottom_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_control_center.bottom_bar, LV_OBJ_FLAG_CLICKABLE);
    
    // Keep the bar always on top
    lv_obj_move_foreground(g_control_center.bottom_bar);
    
    // Create horizontal container for volume controls
    lv_obj_t *volume_container = lv_obj_create(g_control_center.bottom_bar);
    lv_obj_set_size(volume_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(volume_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(volume_container, 0, 0);
    lv_obj_set_style_pad_all(volume_container, 0, 0);
    lv_obj_set_flex_flow(volume_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(volume_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Make container clickable too
    lv_obj_add_flag(volume_container, LV_OBJ_FLAG_CLICKABLE);
    
    // Volume icon/label
    g_control_center.volume_label = lv_label_create(volume_container);
    lv_label_set_text(g_control_center.volume_label, "音量");
    lv_obj_set_style_text_color(g_control_center.volume_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_control_center.volume_label, &yinpin_hm_20, 0);
    lv_obj_set_style_margin_right(g_control_center.volume_label, 10, 0);
    
    // Volume slider
    g_control_center.volume_slider = lv_slider_create(volume_container);
    lv_obj_set_width(g_control_center.volume_slider, 200);
    lv_obj_set_height(g_control_center.volume_slider, 20);
    lv_slider_set_range(g_control_center.volume_slider, 0, 100);
    
    // Set initial value from current audio HAL volume
    uint8_t current_volume = hal_get_speaker_volume();
    lv_slider_set_value(g_control_center.volume_slider, current_volume, LV_ANIM_OFF);
    
    // Style the slider
    lv_obj_set_style_bg_color(g_control_center.volume_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_control_center.volume_slider, lv_color_hex(0x00A8FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_control_center.volume_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_radius(g_control_center.volume_slider, LV_RADIUS_CIRCLE, 0);
    
    // Ensure slider is clickable and can be dragged
    lv_obj_add_flag(g_control_center.volume_slider, LV_OBJ_FLAG_CLICKABLE);
    
    // Add event callback for real-time volume control
    lv_obj_add_event_cb(g_control_center.volume_slider, volume_slider_event, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Volume value label
    g_control_center.value_label = lv_label_create(volume_container);
    lv_label_set_text_fmt(g_control_center.value_label, "%d%%", current_volume);
    lv_obj_set_style_text_color(g_control_center.value_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_control_center.value_label, &yinpin_hm_20, 0);
    lv_obj_set_style_margin_left(g_control_center.value_label, 10, 0);
    lv_obj_set_width(g_control_center.value_label, 40); // Fixed width for consistent layout
    
    // Update value label when slider changes
    lv_obj_add_event_cb(g_control_center.volume_slider, volume_label_update_event, LV_EVENT_VALUE_CHANGED, g_control_center.value_label);
    
    printf("Bottom bar created at position (0, %d) with size (%d x %d)\n", 
           (int)(screen_height - BOTTOM_BAR_HEIGHT), (int)screen_width, BOTTOM_BAR_HEIGHT);
}

void control_center_init(void)
{
    if (g_control_center.is_initialized) {
        return; // Already initialized
    }
    
    create_bottom_bar_ui();
    g_control_center.is_initialized = true;
    
    printf("Control center bottom bar initialized and clickable\n");
}

void control_center_show(void)
{
    // Bottom bar is always visible, so this function does nothing
    // Kept for compatibility with existing code
}

void control_center_hide(void)
{
    // Bottom bar is always visible, so this function does nothing
    // Kept for compatibility with existing code
}

bool control_center_is_visible(void)
{
    return g_control_center.is_initialized;
}

void control_center_deinit(void)
{
    if (!g_control_center.is_initialized) {
        return;
    }
    
    if (g_control_center.bottom_bar) {
        lv_obj_del(g_control_center.bottom_bar);
        g_control_center.bottom_bar = NULL;
        g_control_center.volume_slider = NULL;
        g_control_center.volume_label = NULL;
        g_control_center.value_label = NULL;
    }
    
    g_control_center.is_initialized = false;
}

// New function to get the bottom bar height for other components to adjust their layout
lv_coord_t control_center_get_bar_height(void)
{
    return BOTTOM_BAR_HEIGHT;
}