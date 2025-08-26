#include "hal_display.h"
#include <stdio.h>

// Current brightness level (0-100)
static uint8_t current_brightness = 100;

void hal_display_init(void)
{
    // Initialize display brightness to maximum
    current_brightness = 100;
    hal_display_set_brightness(current_brightness);
    printf("Display HAL initialized with brightness: %d%%\n", current_brightness);
}

void hal_display_set_brightness(uint8_t brightness)
{
    // Clamp brightness to valid range (0-100) using C-style clamping
    if (brightness > 100) {
        brightness = 100;
    }
    
    current_brightness = brightness;
    printf("Setting display brightness to: %d%%\n", current_brightness);
    
    // Use BSP function to set actual brightness
    bsp_display_brightness_set(current_brightness);
}

uint8_t hal_display_get_brightness(void)
{
    return current_brightness;
}

void hal_display_backlight_on(void)
{
    bsp_display_backlight_on();
    printf("Display backlight turned on\n");
}

void hal_display_backlight_off(void)
{
    bsp_display_backlight_off();
    printf("Display backlight turned off\n");
}