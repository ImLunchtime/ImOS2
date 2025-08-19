#include "launcher.h"
#include "window_manager.h"
#include "app_manager.h"
#include "lvgl.h"

static void app_btn_event(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const app_t *app = (const app_t*)lv_event_get_user_data(e);
    if(app && app->launch) {
        app->launch();
    }
}

void launcher_open(void)
{
    // The launcher is non-closable
    wm_window_t *win = wm_open_window("Launcher", false, LV_PCT(45), LV_PCT(40));
    lv_obj_t *content = wm_get_content(win);

    // Grid-style layout (flex wrap)
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(content, 10, 0);
    lv_obj_set_style_pad_column(content, 10, 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);

    size_t count = 0;
    const app_t **apps = app_manager_list(&count);

    for(size_t i = 0; i < count; ++i) {
        const app_t *app = apps[i];
        // Create button for app
        lv_obj_t *btn = lv_btn_create(content);
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
}