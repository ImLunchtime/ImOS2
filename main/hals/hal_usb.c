#include "hals/hal_usb.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>

static const char *TAG = "HAL_USB";

// Global USB mouse data
usb_mouse_data_t g_usb_mouse_data = {0};
// Remove this line - lvUsbMouse is already defined in hal.c:
// lv_indev_t *lvUsbMouse = NULL;

// USB host task handle
static TaskHandle_t usb_host_task_handle = NULL;
static QueueHandle_t app_event_queue = NULL;

// HID device handle
// Remove or comment out this line since it's not used:
// static hid_host_device_handle_t hid_device_handle = NULL;

void hal_usb_init(void)
{
    ESP_LOGI(TAG, "Initializing USB HAL");
    
    // Create mutex for thread-safe access to mouse data
    g_usb_mouse_data.mutex = xSemaphoreCreateMutex();
    if (g_usb_mouse_data.mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mouse data mutex");
        return;
    }
    
    // Initialize mouse position to center of screen using proper LVGL API
    g_usb_mouse_data.x = lv_display_get_horizontal_resolution(NULL) / 2;
    g_usb_mouse_data.y = lv_display_get_vertical_resolution(NULL) / 2;
    
    // Create event queue
    app_event_queue = xQueueCreate(10, sizeof(app_event_queue_t));
    if (app_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return;
    }
    
    // Install USB Host driver
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB host: %s", esp_err_to_name(err));
        return;
    }
    
    // Install HID Host driver
    const hid_host_driver_config_t hid_config = {
        .create_background_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = tskNO_AFFINITY,
        .callback = hid_host_device_event,
        .callback_arg = NULL
    };
    
    err = hid_host_install(&hid_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install HID host: %s", esp_err_to_name(err));
        usb_host_uninstall();
        return;
    }
    
    // Create USB host task
    xTaskCreate(usb_host_task, "usb_host_task", 4096, NULL, 2, &usb_host_task_handle);
    
    ESP_LOGI(TAG, "USB HAL initialized successfully");
}

void hal_usb_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing USB HAL");
    
    // Delete USB host task
    if (usb_host_task_handle != NULL) {
        vTaskDelete(usb_host_task_handle);
        usb_host_task_handle = NULL;
    }
    
    // Uninstall drivers
    hid_host_uninstall();
    usb_host_uninstall();
    
    // Clean up resources
    if (app_event_queue != NULL) {
        vQueueDelete(app_event_queue);
        app_event_queue = NULL;
    }
    
    // Clean up mouse data mutex
    if (g_usb_mouse_data.mutex != NULL) {
        vSemaphoreDelete(g_usb_mouse_data.mutex);
        g_usb_mouse_data.mutex = NULL;
    }
    
    ESP_LOGI(TAG, "USB HAL deinitialized");
}

void hal_usb_mouse_init(void)
{
    ESP_LOGI(TAG, "Initializing USB mouse input device");
    
    // Create LVGL input device for USB mouse
    lvUsbMouse = lv_indev_create();
    lv_indev_set_type(lvUsbMouse, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lvUsbMouse, lvgl_mouse_read_cb);
    
    // Create cursor object from image
    extern const lv_image_dsc_t cursor;
    lv_obj_t *cursor_obj = lv_image_create(lv_screen_active());
    lv_image_set_src(cursor_obj, &cursor);
    lv_obj_add_flag(cursor_obj, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(cursor_obj, LV_OBJ_FLAG_FLOATING);
    
    // Set cursor for the input device
    lv_indev_set_cursor(lvUsbMouse, cursor_obj);
    
    ESP_LOGI(TAG, "USB mouse input device initialized");
}

void usb_host_task(void *arg)
{
    ESP_LOGI(TAG, "USB host task started");
    
    while (1) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_LOGD(TAG, "No more HID clients");
        }
        
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGD(TAG, "All devices freed");
        }
    }
    
    vTaskDelete(NULL);
}

void hid_host_device_event(hid_host_device_handle_t hid_device_handle, const hid_host_driver_event_t event, void *arg)
{
    hid_host_dev_params_t dev_params;
    esp_err_t err = hid_host_device_get_params(hid_device_handle, &dev_params);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get device params: %s", esp_err_to_name(err));
        return;
    }

    switch (event) {
        case HID_HOST_DRIVER_EVENT_CONNECTED:
            ESP_LOGI(TAG, "HID device connected, sub_class: %d, proto: %d", dev_params.sub_class, dev_params.proto);
            
            // Only handle mouse devices
            if (dev_params.sub_class == HID_SUBCLASS_BOOT_INTERFACE && dev_params.proto == HID_PROTOCOL_MOUSE) {
                const hid_host_device_config_t dev_config = {
                    .callback = hid_host_interface_callback,
                    .callback_arg = NULL
                };
                
                err = hid_host_device_open(hid_device_handle, &dev_config);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to open HID device: %s", esp_err_to_name(err));
                    break;
                }
                
                // Set boot protocol for mouse
                err = hid_class_request_set_protocol(hid_device_handle, HID_REPORT_PROTOCOL_BOOT);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to set boot protocol: %s", esp_err_to_name(err));
                }
                
                err = hid_host_device_start(hid_device_handle);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to start HID device: %s", esp_err_to_name(err));
                }
            }
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown HID device event: %d", event);
            break;
    }
}

void hid_host_interface_callback(hid_host_device_handle_t hid_device_handle, const hid_host_interface_event_t event, void *arg)
{
    uint8_t data[64] = {0};
    size_t data_length = 0;
    hid_host_dev_params_t dev_params;
    esp_err_t err;

    switch (event) {
        case HID_HOST_INTERFACE_EVENT_INPUT_REPORT:
            err = hid_host_device_get_raw_input_report_data(hid_device_handle, data, sizeof(data), &data_length);
            if (err == ESP_OK && data_length > 0) {
                err = hid_host_device_get_params(hid_device_handle, &dev_params);
                if (err == ESP_OK && dev_params.sub_class == HID_SUBCLASS_BOOT_INTERFACE && dev_params.proto == HID_PROTOCOL_MOUSE) {
                    hid_host_mouse_report_callback(data, data_length);
                }
            }
            break;
            
        case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HID device disconnected");
            esp_err_t err = hid_host_device_close(hid_device_handle);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to close HID device: %s", esp_err_to_name(err));
            }
            
            // Reset mouse data safely
            if (xSemaphoreTake(g_usb_mouse_data.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                g_usb_mouse_data.x = lv_display_get_horizontal_resolution(NULL) / 2;
                g_usb_mouse_data.y = lv_display_get_vertical_resolution(NULL) / 2;
                g_usb_mouse_data.left_button = false;
                g_usb_mouse_data.right_button = false;
                g_usb_mouse_data.middle_button = false;
                g_usb_mouse_data.wheel = 0;
                xSemaphoreGive(g_usb_mouse_data.mutex);
            }
            break;
            
        case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
            ESP_LOGW(TAG, "HID transfer error");
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown HID interface event: %d", event);
            break;
    }
}

void hid_host_mouse_report_callback(const uint8_t *const data, const int length)
{
    if (data == NULL || length < 3) {
        return;
    }
    
    // Parse mouse report (standard HID mouse report format)
    // Byte 0: Button states
    // Byte 1: X movement (signed)
    // Byte 2: Y movement (signed)
    // Byte 3: Wheel movement (signed, optional)
    
    if (xSemaphoreTake(g_usb_mouse_data.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        // Update button states
        g_usb_mouse_data.left_button = (data[0] & 0x01) != 0;
        g_usb_mouse_data.right_button = (data[0] & 0x02) != 0;
        g_usb_mouse_data.middle_button = (data[0] & 0x04) != 0;
        
        // Fix: Swap X and Y, and invert Y axis for proper screen mapping
        // Original: X += data[1], Y += data[2]
        // Fixed: X += data[2], Y -= data[1] (inverted Y)
        g_usb_mouse_data.x += (int8_t)data[2];  // Use Y movement for X
        g_usb_mouse_data.y -= (int8_t)data[1];  // Use X movement for Y (inverted)
        
        // Clamp coordinates to screen bounds using proper LVGL API
        // IMPORTANT: Since we swapped X/Y axes, we need to clamp accordingly:
        // - g_usb_mouse_data.x (which uses data[2]/Y movement) should be clamped to screen HEIGHT
        // - g_usb_mouse_data.y (which uses data[1]/X movement) should be clamped to screen WIDTH
        lv_coord_t screen_width = lv_display_get_horizontal_resolution(NULL);
        lv_coord_t screen_height = lv_display_get_vertical_resolution(NULL);
        
        if (g_usb_mouse_data.x < 0) g_usb_mouse_data.x = 0;
        if (g_usb_mouse_data.y < 0) g_usb_mouse_data.y = 0;
        if (g_usb_mouse_data.x >= screen_height) g_usb_mouse_data.x = screen_height - 1;  // X uses height bounds
        if (g_usb_mouse_data.y >= screen_width) g_usb_mouse_data.y = screen_width - 1;    // Y uses width bounds
        
        // Update wheel if available
        if (length > 3) {
            g_usb_mouse_data.wheel = (int8_t)data[3];
        }
        
        xSemaphoreGive(g_usb_mouse_data.mutex);
        
        ESP_LOGD(TAG, "Mouse: x=%d, y=%d, buttons=0x%02X, wheel=%d", 
                 g_usb_mouse_data.x, g_usb_mouse_data.y, data[0], g_usb_mouse_data.wheel);
    }
}

void lvgl_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (xSemaphoreTake(g_usb_mouse_data.mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
        // Set cursor position
        data->point.x = g_usb_mouse_data.x;
        data->point.y = g_usb_mouse_data.y;
        
        // Set button state (LVGL uses left button for primary input)
        data->state = g_usb_mouse_data.left_button ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        
        xSemaphoreGive(g_usb_mouse_data.mutex);
    } else {
        // Fallback if mutex is not available
        data->state = LV_INDEV_STATE_REL;
    }
}