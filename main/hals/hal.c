#include "hals/hal.h"
#include <stdio.h>

lv_disp_t *lvDisp = NULL;
lv_indev_t *lvTouchpad = NULL;
lv_indev_t *lvUsbMouse = NULL;

extern esp_lcd_touch_handle_t _lcd_touch_handle;

static void lvgl_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (_lcd_touch_handle == NULL)
    {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    uint16_t touch_x[1];
    uint16_t touch_y[1];
    uint16_t touch_strength[1];
    uint8_t touch_cnt = 0;

    esp_lcd_touch_read_data(_lcd_touch_handle);
    bool touchpad_pressed =
        esp_lcd_touch_get_coordinates(_lcd_touch_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);

    if (!touchpad_pressed)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touch_x[0];
        data->point.y = touch_y[0];
    }
}

void hal_init(void)
{
    // Initialize I2C bus
    bsp_i2c_init();
    vTaskDelay(pdMS_TO_TICKS(200));

    // Get I2C bus handle and initialize IO expander
    i2c_master_bus_handle_t i2c_bus_handle = bsp_i2c_get_handle();
    bsp_io_expander_pi4ioe_init(i2c_bus_handle);

    // Initialize audio
    hal_audio_init();
    // Initialize SD card
    hal_sdcard_init();

    // Initialize display and touch
    bsp_reset_tp();
    // Initialize display with custom config for larger stack
    bsp_display_cfg_t display_cfg = {
        .lvgl_port_cfg = {
            .task_priority = 4,
            .task_stack = 16384,  // Increase LVGL task stack size
            .task_affinity = -1,
            .task_max_sleep_ms = 500,
            .timer_period_ms = 5,
        },
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = true,
        }
    };
    
    lvDisp = bsp_display_start_with_config(&display_cfg);
    lv_display_set_rotation(lvDisp, LV_DISPLAY_ROTATION_90);
    
    // Initialize display HAL (this will turn on backlight and set initial brightness)
    hal_display_init();
    
    // Initialize USB HAL
    hal_usb_init();
}

void hal_touchpad_init(void)
{
    // Initialize touchpad input
    lvTouchpad = lv_indev_create();
    lv_indev_set_type(lvTouchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lvTouchpad, lvgl_read_cb);
    lv_indev_set_display(lvTouchpad, lvDisp);
    
    // Initialize USB mouse input
    hal_usb_mouse_init();
}