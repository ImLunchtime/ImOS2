#pragma once

#include "lvgl.h"
#include <stdbool.h>

typedef struct wm_window_t wm_window_t;

// Open a new window (pop-up style). If width/height are 0, defaults are used.
// Windows cannot be moved. If closable is false, tapping outside will not close it.
wm_window_t* wm_open_window(const char *title, bool closable, lv_coord_t width, lv_coord_t height);

// Get the content container for adding widgets into the window.
lv_obj_t* wm_get_content(wm_window_t *win);

// Close the topmost window if it is closable. Returns true if closed.
bool wm_close_top(void);

// Get the current top window (may be NULL).
wm_window_t* wm_top(void);

// Number of windows currently open.
int wm_count(void);