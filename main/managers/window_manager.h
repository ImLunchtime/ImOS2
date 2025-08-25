#pragma once

#include "lvgl.h"
#include <stdbool.h>

typedef struct wm_window_t wm_window_t;

// Window positioning options
typedef enum {
    WM_POS_DEFAULT,     // Default stacked positioning
    WM_POS_BOTTOM,      // Bottom of screen
    WM_POS_TOP,         // Top of screen
    WM_POS_CENTER,      // Center of screen
    WM_POS_CUSTOM       // Custom x,y coordinates
} wm_position_t;

typedef struct {
    wm_position_t pos_type;
    lv_coord_t x;       // Used for WM_POS_CUSTOM
    lv_coord_t y;       // Used for WM_POS_CUSTOM
    lv_coord_t offset_x; // Additional offset for predefined positions
    lv_coord_t offset_y; // Additional offset for predefined positions
} wm_position_config_t;

// Open a new window (pop-up style). If width/height are 0, defaults are used.
// Windows cannot be moved. If closable is false, tapping outside will not close it.
wm_window_t* wm_open_window(const char *title, bool closable, lv_coord_t width, lv_coord_t height);

// Open a window with custom positioning
wm_window_t* wm_open_window_positioned(const char *title, bool closable, lv_coord_t width, lv_coord_t height, const wm_position_config_t *pos_config);

// Open a window with custom background color
wm_window_t* wm_open_window_with_color(const char *title, bool closable, lv_coord_t width, lv_coord_t height, lv_color_t bg_color);

// Open a window with custom positioning and background color
wm_window_t* wm_open_window_positioned_with_color(const char *title, bool closable, lv_coord_t width, lv_coord_t height, const wm_position_config_t *pos_config, lv_color_t bg_color);

// Get the content container for adding widgets into the window.
lv_obj_t* wm_get_content(wm_window_t *win);

// Close the topmost window if it is closable. Returns true if closed.
bool wm_close_top(void);

// Get the current top window (may be NULL).
wm_window_t* wm_top(void);

// Number of windows currently open.
int wm_count(void);