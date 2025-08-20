#include "apps/music/music.h"
#include "managers/window_manager.h"
#include "lvgl.h"

static void music_launch(void)
{
    wm_window_t *win = wm_open_window("Music", true, LV_PCT(60), LV_PCT(65));
    lv_obj_t *content = wm_get_content(win);

    lv_obj_t *lbl = lv_label_create(content);
    lv_label_set_text(lbl, "Music (placeholder)");
    lv_obj_center(lbl);
}

const app_t APP_MUSIC = {
    .id = "music",
    .name = "Music",
    .launch = music_launch
};