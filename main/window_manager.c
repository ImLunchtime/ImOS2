#include "window_manager.h"
#include <string.h>

#ifndef WM_MAX_WINDOWS
#define WM_MAX_WINDOWS 8
#endif

typedef struct wm_window_t {
    lv_obj_t *overlay;
    lv_obj_t *panel;
    lv_obj_t *content;
    bool closable;
} wm_window_t;

static wm_window_t* s_stack[WM_MAX_WINDOWS];
static int s_stack_sz = 0;

static void panel_click_handler(lv_event_t *e)
{
    // Stop bubbling so overlay won't receive click if panel was clicked
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_event_stop_bubbling(e);
    }
}

static void overlay_click_handler(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    // Attempt to close only if this overlay belongs to the top window
    if(s_stack_sz <= 0) return;

    wm_window_t *top = s_stack[s_stack_sz - 1];
    lv_obj_t *target = lv_event_get_target(e);

    if(top && target == top->overlay && top->closable) {
        lv_obj_del(top->overlay); // deletes children (panel/content) as well
        lv_free(top);
        s_stack_sz--;
        return;
    }
}

static wm_window_t* wm_create_window_internal(const char *title, bool closable, lv_coord_t width, lv_coord_t height)
{
    if(s_stack_sz >= WM_MAX_WINDOWS) return NULL;

    wm_window_t *win = (wm_window_t*)lv_malloc(sizeof(wm_window_t));
    if(!win) return NULL;
    memset(win, 0, sizeof(*win));
    win->closable = closable;

    lv_obj_t *scr = lv_scr_act();

    // Create overlay (transparent, still clickable for outside-tap-to-close)
    win->overlay = lv_obj_create(scr);
    lv_obj_remove_style_all(win->overlay);
    lv_obj_set_size(win->overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(win->overlay, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(win->overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(win->overlay, overlay_click_handler, LV_EVENT_CLICKED, NULL);

    // Create panel (the window) as a child of overlay
    win->panel = lv_obj_create(win->overlay);
    /* Liquid-Glass panel style */
    lv_obj_set_style_radius(win->panel, 20, 0);
    lv_obj_set_style_pad_all(win->panel, 12, 0);
    lv_obj_set_style_bg_color(win->panel, lv_color_hex(0xf6f6f6), 0);
    lv_obj_set_style_bg_opa(win->panel, LV_OPA_100, 0); /* not translucent */
    lv_obj_set_style_border_width(win->panel, 1, 0);
    lv_obj_set_style_border_color(win->panel, lv_color_white(), 0);
    /* 添加阴影效果 */
    lv_obj_set_style_shadow_width(win->panel, 20, 0);
    lv_obj_set_style_shadow_spread(win->panel, 2, 0);
    lv_obj_set_style_shadow_ofs_x(win->panel, 0, 0);
    lv_obj_set_style_shadow_ofs_y(win->panel, 5, 0);
    lv_obj_set_style_shadow_opa(win->panel, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(win->panel, lv_palette_darken(LV_PALETTE_GREY, 2), 0);

    lv_obj_add_flag(win->panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(win->panel, panel_click_handler, LV_EVENT_CLICKED, NULL);

    // Default size if not provided (still never full-screen)
    if(width == 0)  width = LV_PCT(80);
    if(height == 0) height = LV_PCT(70);
    lv_obj_set_size(win->panel, width, height);

    // Title (top label)
    if(title && title[0]) {
        lv_obj_t *title_label = lv_label_create(win->panel);
        lv_label_set_text(title_label, title);
        lv_obj_set_style_text_font(title_label, lv_theme_get_font_large(win->panel), 0);
        lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 4, 0);
    }

    // Content container
    win->content = lv_obj_create(win->panel);
    lv_obj_remove_style_all(win->content);
    lv_obj_set_size(win->content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_top(win->content, 28, 0);   // leave space for title
    lv_obj_set_style_pad_left(win->content, 8, 0);
    lv_obj_set_style_pad_right(win->content, 8, 0);
    lv_obj_set_style_pad_bottom(win->content, 8, 0);
    lv_obj_set_flex_flow(win->content, LV_FLEX_FLOW_COLUMN);

    // Position: top-left; offset +20,+20 from parent (previous top) if exists
    lv_coord_t x = 30, y = 30;
    if(s_stack_sz > 0) {
        lv_obj_t *parent_panel = s_stack[s_stack_sz - 1]->panel;
        x = lv_obj_get_x(parent_panel) + 70;
        y = lv_obj_get_y(parent_panel) + 70;
    }
    lv_obj_align(win->panel, LV_ALIGN_TOP_LEFT, x, y);

    // Push to stack (topmost)
    s_stack[s_stack_sz++] = win;

    return win;
}

wm_window_t* wm_open_window(const char *title, bool closable, lv_coord_t width, lv_coord_t height)
{
    return wm_create_window_internal(title, closable, width, height);
}

lv_obj_t* wm_get_content(wm_window_t *win)
{
    if(!win) return NULL;
    return win->content;
}

bool wm_close_top(void)
{
    if(s_stack_sz <= 0) return false;
    wm_window_t *top = s_stack[s_stack_sz - 1];
    if(!top->closable) return false;

    lv_obj_del(top->overlay);
    lv_free(top);
    s_stack_sz--;
    return true;
}

wm_window_t* wm_top(void)
{
    if(s_stack_sz <= 0) return NULL;
    return s_stack[s_stack_sz - 1];
}

int wm_count(void)
{
    return s_stack_sz;
}