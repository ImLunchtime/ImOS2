#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <stdint.h>
#include <bsp/esp-bsp.h>
#include "lvgl.h"

/**
 * @brief Initialize the display HAL
 */
void hal_display_init(void);

/**
 * @brief Set display brightness
 * @param brightness Brightness level (0-100)
 */
void hal_display_set_brightness(uint8_t brightness);

/**
 * @brief Get current display brightness
 * @return Current brightness level (0-100)
 */
uint8_t hal_display_get_brightness(void);

/**
 * @brief Turn display backlight on
 */
void hal_display_backlight_on(void);

/**
 * @brief Turn display backlight off
 */
void hal_display_backlight_off(void);

#endif // HAL_DISPLAY_H