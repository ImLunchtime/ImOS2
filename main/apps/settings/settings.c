#include "apps/settings/settings.h"
#include "managers/window_manager.h"
#include "lvgl.h"

// Menu item builder variants
typedef enum {
    LV_MENU_ITEM_BUILDER_VARIANT_1,
    LV_MENU_ITEM_BUILDER_VARIANT_2
} lv_menu_builder_variant_t;

// Static variables for menu management
static lv_obj_t *settings_menu = NULL;
static lv_obj_t *root_page = NULL;
static wm_window_t *settings_window = NULL;

// Forward declarations
static void back_event_handler(lv_event_t *e);
static void switch_handler(lv_event_t *e);
static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt, lv_menu_builder_variant_t builder_variant);
static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk);
static void create_settings_menu(lv_obj_t *parent);

static void settings_launch(void)
{
    // Open a window for the settings app
    settings_window = wm_open_window("Settings", true, LV_PCT(70), LV_PCT(70));
    lv_obj_t *content = wm_get_content(settings_window);
    
    // Create the settings menu inside the window
    create_settings_menu(content);
}

static void create_settings_menu(lv_obj_t *parent)
{
    // Create the main menu
    settings_menu = lv_menu_create(parent);
    
    // Style the menu background
    lv_color_t bg_color = lv_obj_get_style_bg_color(settings_menu, 0);
    if(lv_color_brightness(bg_color) > 127) {
        lv_obj_set_style_bg_color(settings_menu, lv_color_darken(lv_obj_get_style_bg_color(settings_menu, 0), 10), 0);
    } else {
        lv_obj_set_style_bg_color(settings_menu, lv_color_darken(lv_obj_get_style_bg_color(settings_menu, 0), 50), 0);
    }
    
    // Enable back button and set event handler
    lv_menu_set_mode_root_back_button(settings_menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(settings_menu, back_event_handler, LV_EVENT_CLICKED, settings_menu);
    
    // Set menu size to fill the parent
    lv_obj_set_size(settings_menu, LV_PCT(100), LV_PCT(100));
    
    lv_obj_t *cont;
    lv_obj_t *section;
    
    // Create Display settings page
    lv_obj_t *sub_display_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_display_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    lv_menu_separator_create(sub_display_page);
    section = lv_menu_section_create(sub_display_page);
    create_slider(section, LV_SYMBOL_SETTINGS, "Brightness", 0, 100, 75);
    create_switch(section, LV_SYMBOL_EYE_OPEN, "Auto Brightness", false);
    create_slider(section, LV_SYMBOL_SETTINGS, "Screen Timeout", 10, 300, 60);
    
    // Create Sound settings page
    lv_obj_t *sub_sound_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_sound_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    lv_menu_separator_create(sub_sound_page);
    section = lv_menu_section_create(sub_sound_page);
    create_switch(section, LV_SYMBOL_AUDIO, "Sound", true);
    create_slider(section, LV_SYMBOL_VOLUME_MAX, "Volume", 0, 100, 50);
    create_switch(section, LV_SYMBOL_BELL, "Notifications", true);
    
    // Create System settings page
    lv_obj_t *sub_system_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_system_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    lv_menu_separator_create(sub_system_page);
    section = lv_menu_section_create(sub_system_page);
    create_text(section, LV_SYMBOL_SETTINGS, "Device Name: M5Stack Tab5", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, LV_SYMBOL_WIFI, "WiFi Settings", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, LV_SYMBOL_BLUETOOTH, "Bluetooth Settings", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_switch(section, LV_SYMBOL_POWER, "Power Saving", false);
    
    // Create Software Info page
    lv_obj_t *sub_software_info_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_software_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    section = lv_menu_section_create(sub_software_info_page);
    create_text(section, NULL, "Firmware Version: 1.0.0", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "LVGL Version: 9.x", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "ESP-IDF Version: 5.x", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "Build Date: " __DATE__, LV_MENU_ITEM_BUILDER_VARIANT_1);
    
    // Create Legal Info page
    lv_obj_t *sub_legal_info_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_legal_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    section = lv_menu_section_create(sub_legal_info_page);
    create_text(section, NULL, "Copyright © 2024 M5Stack", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "Licensed under MIT License", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "LVGL is licensed under MIT License", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "ESP-IDF is licensed under Apache 2.0", LV_MENU_ITEM_BUILDER_VARIANT_1);
    
    // Create About page
    lv_obj_t *sub_about_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_about_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    lv_menu_separator_create(sub_about_page);
    section = lv_menu_section_create(sub_about_page);
    cont = create_text(section, NULL, "Software Information", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_software_info_page);
    cont = create_text(section, NULL, "Legal Information", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_legal_info_page);
    
    // Create Menu Mode page
    lv_obj_t *sub_menu_mode_page = lv_menu_page_create(settings_menu, NULL);
    lv_obj_set_style_pad_hor(sub_menu_mode_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    lv_menu_separator_create(sub_menu_mode_page);
    section = lv_menu_section_create(sub_menu_mode_page);
    cont = create_switch(section, LV_SYMBOL_LIST, "Sidebar Enable", true);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), switch_handler, LV_EVENT_VALUE_CHANGED, settings_menu);
    
    // Create the root page with sidebar
    root_page = lv_menu_page_create(settings_menu, "Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(settings_menu), 0), 0);
    
    // Main settings section
    section = lv_menu_section_create(root_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Display", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_display_page);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Sound", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_sound_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "System", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_system_page);
    
    // Other settings section
    create_text(root_page, NULL, "Other", LV_MENU_ITEM_BUILDER_VARIANT_1);
    section = lv_menu_section_create(root_page);
    cont = create_text(section, LV_SYMBOL_FILE, "About", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_about_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Menu Mode", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(settings_menu, cont, sub_menu_mode_page);
    
    // Set the sidebar page
    lv_menu_set_sidebar_page(settings_menu, root_page);
    
    // Trigger initial click to show the first page
    lv_obj_send_event(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(settings_menu), 0), 0), LV_EVENT_CLICKED, NULL);
}

static void back_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *menu = lv_event_get_user_data(e);
    
    if(lv_menu_back_button_is_root(menu, obj)) {
        // Close the settings window when back is pressed on root
        if(settings_window) {
            wm_close_top();
            settings_window = NULL;
            settings_menu = NULL;
        }
    }
}

static void switch_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *menu = lv_event_get_user_data(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            // Enable sidebar mode
            lv_menu_set_page(menu, NULL);
            lv_menu_set_sidebar_page(menu, root_page);
            lv_obj_send_event(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
        } else {
            // Disable sidebar mode
            lv_menu_set_sidebar_page(menu, NULL);
            lv_menu_clear_history(menu);
            lv_menu_set_page(menu, root_page);
        }
    }
}

static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt, lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t *obj = lv_menu_cont_create(parent);
    
    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;
    
    if(icon) {
        img = lv_image_create(obj);
        lv_image_set_src(img, icon);
    }
    
    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }
    
    if(builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }
    
    return obj;
}

static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max, int32_t val)
{
    lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);
    
    lv_obj_t *slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);
    
    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }
    
    return obj;
}

static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk)
{
    lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);
    
    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);
    
    return obj;
}

const app_t APP_SETTINGS = {
    .id = "settings",
    .name = "Settings",
    .launch = settings_launch
};