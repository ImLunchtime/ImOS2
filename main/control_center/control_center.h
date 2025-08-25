#pragma once

#include "lvgl.h"
#include <stdbool.h>

// Initialize the control center bottom bar
void control_center_init(void);

// Show control center (no-op for bottom bar, kept for compatibility)
void control_center_show(void);

// Hide control center (no-op for bottom bar, kept for compatibility)
void control_center_hide(void);

// Check if control center is visible (always true for bottom bar)
bool control_center_is_visible(void);

// Cleanup control center
void control_center_deinit(void);

// Get the height of the bottom bar for layout calculations
lv_coord_t control_center_get_bar_height(void);