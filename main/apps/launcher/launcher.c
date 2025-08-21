#include "apps/launcher/launcher.h"
#include "managers/window_manager.h"
#include "managers/app_manager.h"
#include "hals/hal_audio.h"
#include "lvgl.h"

static void app_btn_event(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const app_t *app = (const app_t*)lv_event_get_user_data(e);
    if(app && app->launch) {
        app->launch();
    }
}

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

void launcher_open(void)
{
    // The launcher is non-closable
    wm_window_t *win = wm_open_window("Launcher", false, LV_PCT(45), LV_PCT(50));
    lv_obj_t *content = wm_get_content(win);

    // Main container with column layout
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(content, 10, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);

    // App buttons container
    lv_obj_t *apps_container = lv_obj_create(content);
    lv_obj_set_width(apps_container, LV_PCT(100));
    lv_obj_set_flex_grow(apps_container, 1);
    lv_obj_set_style_bg_opa(apps_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(apps_container, 0, 0);
    lv_obj_set_style_pad_all(apps_container, 0, 0);
    
    // Grid-style layout (flex wrap) for app buttons
    lv_obj_set_flex_flow(apps_container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(apps_container, 10, 0);
    lv_obj_set_style_pad_column(apps_container, 10, 0);

    size_t count = 0;
    const app_t **apps = app_manager_list(&count);

    for(size_t i = 0; i < count; ++i) {
        const app_t *app = apps[i];
        // Create button for app
        lv_obj_t *btn = lv_btn_create(apps_container);
        lv_obj_set_size(btn, 110, 55);               /* 2:1 pill ratio */
        /* Liquid-Glass button style */
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_add_event_cb(btn, app_btn_event, LV_EVENT_CLICKED, (void*)app);

        // Centered label
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, app->name ? app->name : "App");
        lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
        lv_obj_center(lbl);
    }
    
    // Volume control section at the bottom
    lv_obj_t *volume_container = lv_obj_create(content);
    lv_obj_set_width(volume_container, LV_PCT(100));
    lv_obj_set_height(volume_container, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(volume_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(volume_container, 0, 0);
    lv_obj_set_style_pad_all(volume_container, 10, 0);
    lv_obj_set_flex_flow(volume_container, LV_FLEX_FLOW_COLUMN);
    
    // Volume label
    lv_obj_t *volume_label = lv_label_create(volume_container);
    lv_label_set_text(volume_label, "Volume");
    lv_obj_set_style_text_color(volume_label, lv_color_white(), 0);
    lv_obj_set_style_margin_bottom(volume_label, 5, 0);
    
    // Volume slider
    lv_obj_t *volume_slider = lv_slider_create(volume_container);
    lv_obj_set_width(volume_slider, LV_PCT(90));
    lv_obj_set_height(volume_slider, 20);
    lv_slider_set_range(volume_slider, 0, 100);
    
    // Set initial value from current audio HAL volume
    uint8_t current_volume = hal_get_speaker_volume();
    lv_slider_set_value(volume_slider, current_volume, LV_ANIM_OFF);
    
    // Style the slider
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x00A8FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(volume_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_radius(volume_slider, LV_RADIUS_CIRCLE, 0);
    
    // Add event callback for real-time volume control
    lv_obj_add_event_cb(volume_slider, volume_slider_event, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Volume value label
    lv_obj_t *value_label = lv_label_create(volume_container);
    lv_label_set_text_fmt(value_label, "%d%%", current_volume);
    lv_obj_set_style_text_color(value_label, lv_color_white(), 0);
    lv_obj_set_style_margin_top(value_label, 5, 0);
    
    // Update value label when slider changes
    lv_obj_add_event_cb(volume_slider, volume_label_update_event, LV_EVENT_VALUE_CHANGED, value_label);
}