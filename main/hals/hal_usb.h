#ifndef HAL_USB_H
#define HAL_USB_H

#include "lvgl.h"
#include "usb/usb_host.h"
#include "usb/hid_host.h"
#include "usb/hid_usage_mouse.h"
#include "usb/hid_usage_keyboard.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// USB mouse data structure
typedef struct {
    int16_t x;
    int16_t y;
    bool left_button;
    bool right_button;
    bool middle_button;
    int8_t wheel;
    SemaphoreHandle_t mutex;
} usb_mouse_data_t;

// Event queue structure for USB events
typedef struct {
    int event_type;
    void *data;
} app_event_queue_t;

// Global USB mouse data
extern usb_mouse_data_t g_usb_mouse_data;
extern lv_indev_t *lvUsbMouse;

// USB HAL functions
void hal_usb_init(void);
void hal_usb_deinit(void);
void hal_usb_mouse_init(void);

// Internal USB functions
void usb_host_task(void *arg);
void hid_host_interface_callback(hid_host_device_handle_t hid_device_handle, const hid_host_interface_event_t event, void *arg);
void hid_host_device_event(hid_host_device_handle_t hid_device_handle, const hid_host_driver_event_t event, void *arg);
void hid_host_mouse_report_callback(const uint8_t *const data, const int length);
void lvgl_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data);

#ifdef __cplusplus
}
#endif

#endif // HAL_USB_H