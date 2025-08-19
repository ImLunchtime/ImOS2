#include "settings.h"
#include "window_manager.h"
#include "lvgl.h"

static void settings_launch(void)
{
    wm_window_t *win = wm_open_window("Settings", true, LV_PCT(70), LV_PCT(70));
    lv_obj_t *content = wm_get_content(win);

    lv_obj_t *lbl = lv_label_create(content);
    lv_label_set_text(lbl, "Settings (placeholder)");
    lv_obj_center(lbl);
}

const app_t APP_SETTINGS = {
    .id = "settings",
    .name = "Settings",
    .launch = settings_launch
};