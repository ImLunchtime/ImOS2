#ifndef HAL_H
#define HAL_H

#include <bsp/esp-bsp.h>
#include "lvgl.h"
#include "hals/hal_audio.h"
#include "hals/hal_sdcard.h"
#include "hals/hal_display.h"
#include "hals/hal_usb.h"

// Display and input device handles
extern lv_disp_t *lvDisp;
extern lv_indev_t *lvTouchpad;
extern lv_indev_t *lvUsbMouse;

// HAL initialization functions
void hal_init(void);
void hal_touchpad_init(void);
void hal_usb_init(void);
void hal_usb_mouse_init(void);

#endif // HAL_H