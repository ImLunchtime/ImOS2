#include "apps/file_manager/file_manager.h"
#include "managers/window_manager.h"
#include "hals/hal_sdcard.h"
#include "theme/theme_engine.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// File manager state
static struct {
    wm_window_t *window;
    lv_obj_t *file_list;
    lv_obj_t *path_label;
    lv_obj_t *status_label;
    char current_path[256];
} fm_state = {0};

// Forward declarations
static void file_manager_launch(void);
static void create_file_manager_ui(lv_obj_t *parent);
static void refresh_file_list(void);
static void file_item_event_cb(lv_event_t *e);
static void back_btn_event_cb(lv_event_t *e);
static void refresh_btn_event_cb(lv_event_t *e);
static bool is_directory(const char *path);
static void navigate_to_directory(const char *path);
static void show_file_info(const char *filepath);

static void file_manager_launch(void)
{
    // Initialize current path
    strcpy(fm_state.current_path, "/sdcard");
    
    // Open window with Chinese title
    fm_state.window = wm_open_window("文件管理器", true, LV_PCT(80), LV_PCT(80));
    if (!fm_state.window) {
        printf("Failed to create file manager window\n");
        return;
    }
    
    // Get window content area
    lv_obj_t *content = wm_get_content(fm_state.window);
    if (!content) {
        printf("Failed to get window content\n");
        return;
    }
    
    // Create the file manager UI
    create_file_manager_ui(content);
    
    // Initial file list refresh
    refresh_file_list();
}

static void create_file_manager_ui(lv_obj_t *parent)
{
    // Main container with column layout
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, 0);
    
    // Top toolbar container
    lv_obj_t *toolbar = lv_obj_create(parent);
    lv_obj_set_width(toolbar, LV_PCT(100));
    lv_obj_set_height(toolbar, 50);
    lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(toolbar, 5, 0);
    lv_obj_set_style_bg_opa(toolbar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(toolbar, 0, 0);
    
    // Back button
    lv_obj_t *back_btn = lv_btn_create(toolbar);
    lv_obj_set_size(back_btn, 60, 40);
    
    // Apply theme button styling
    theme_apply_button_style(back_btn);
    // Override size since theme sets 110x55
    lv_obj_set_size(back_btn, 60, 40);
    
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    theme_apply_button_icon_style(back_label);
    
    // Refresh button
    lv_obj_t *refresh_btn = lv_btn_create(toolbar);
    lv_obj_set_size(refresh_btn, 60, 40);
    
    // Apply theme button styling
    theme_apply_button_style(refresh_btn);
    // Override size since theme sets 110x55
    lv_obj_set_size(refresh_btn, 60, 40);
    
    lv_obj_add_event_cb(refresh_btn, refresh_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *refresh_label = lv_label_create(refresh_btn);
    lv_label_set_text(refresh_label, LV_SYMBOL_REFRESH);
    theme_apply_button_icon_style(refresh_label);
    lv_obj_center(refresh_label);
    
    // Path label (flexible width)
    fm_state.path_label = lv_label_create(toolbar);
    lv_label_set_text(fm_state.path_label, fm_state.current_path);
    
    // Apply theme label styling
    theme_apply_label_style(fm_state.path_label);
    // Override text alignment for path display
    lv_obj_set_style_text_align(fm_state.path_label, LV_TEXT_ALIGN_LEFT, 0);
    
    lv_obj_set_flex_grow(fm_state.path_label, 1);
    lv_label_set_long_mode(fm_state.path_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    
    // File list container
    lv_obj_t *list_container = lv_obj_create(parent);
    lv_obj_set_width(list_container, LV_PCT(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_style_pad_all(list_container, 5, 0);
    
    // File list
    fm_state.file_list = lv_list_create(list_container);
    lv_obj_set_size(fm_state.file_list, LV_PCT(100), LV_PCT(100));
    
    // Status label at bottom
    fm_state.status_label = lv_label_create(parent);
    lv_label_set_text(fm_state.status_label, "就绪");
    
    // Apply theme label styling
    theme_apply_label_style(fm_state.status_label);
    
    lv_obj_set_width(fm_state.status_label, LV_PCT(100));
}

static void refresh_file_list(void)
{
    if (!fm_state.file_list) return;
    
    // Clear existing list items
    lv_obj_clean(fm_state.file_list);
    
    // Update path label
    lv_label_set_text(fm_state.path_label, fm_state.current_path);
    
    // Check if SD card is mounted
    if (!hal_sdcard_is_mounted()) {
        lv_label_set_text(fm_state.status_label, "SD Card not mounted");
        lv_obj_t *item = lv_list_add_text(fm_state.file_list, "SD Card not available");
        lv_obj_set_style_text_color(item, lv_palette_main(LV_PALETTE_RED), 0);
        return;
    }
    
    // Open directory
    DIR *dir = opendir(fm_state.current_path);
    if (!dir) {
        lv_label_set_text(fm_state.status_label, "Cannot open directory");
        return;
    }
    
    struct dirent *entry;
    int file_count = 0;
    int dir_count = 0;
    
    // Add parent directory entry if not at root
    if (strcmp(fm_state.current_path, "/") != 0 && 
        strcmp(fm_state.current_path, hal_sdcard_get_mount_point()) != 0) {
        lv_obj_t *item = lv_list_add_button(fm_state.file_list, LV_SYMBOL_DIRECTORY, "..");
        lv_obj_add_event_cb(item, file_item_event_cb, LV_EVENT_CLICKED, (void*)"..");
    }
    
    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files and current directory
        if (entry->d_name[0] == '.') continue;
        
        // Build full path
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", fm_state.current_path, entry->d_name);
        
        // Check if it's a directory
        bool is_dir = is_directory(full_path);
        
        // Create list item
        const char *icon = is_dir ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE;
        lv_obj_t *item = lv_list_add_button(fm_state.file_list, icon, entry->d_name);
        
        // Store the filename as user data (allocate memory for it)
        char *filename = malloc(strlen(entry->d_name) + 1);
        strcpy(filename, entry->d_name);
        lv_obj_add_event_cb(item, file_item_event_cb, LV_EVENT_CLICKED, filename);
        
        // Count files and directories
        if (is_dir) {
            dir_count++;
        } else {
            file_count++;
        }
    }
    
    closedir(dir);
    
    // Update status
    char status_text[100];
    snprintf(status_text, sizeof(status_text), "共%d个目录, %d个文件", dir_count, file_count);
    lv_label_set_text(fm_state.status_label, status_text);
}

static void file_item_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    
    const char *filename = (const char*)lv_event_get_user_data(e);
    if (!filename) return;
    
    // Handle parent directory
    if (strcmp(filename, "..") == 0) {
        // Navigate to parent directory
        char *last_slash = strrchr(fm_state.current_path, '/');
        if (last_slash && last_slash != fm_state.current_path) {
            *last_slash = '\0';
        } else {
            strcpy(fm_state.current_path, hal_sdcard_get_mount_point());
        }
        refresh_file_list();
        return;
    }
    
    // Build full path
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", fm_state.current_path, filename);
    
    if (is_directory(full_path)) {
        // Navigate to directory
        navigate_to_directory(full_path);
    } else {
        // Show file info
        show_file_info(full_path);
    }
}

static void back_btn_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    
    // Navigate to parent directory
    char *last_slash = strrchr(fm_state.current_path, '/');
    if (last_slash && last_slash != fm_state.current_path) {
        *last_slash = '\0';
    } else if (hal_sdcard_is_mounted()) {
        strcpy(fm_state.current_path, hal_sdcard_get_mount_point());
    }
    refresh_file_list();
}

static void refresh_btn_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    refresh_file_list();
}

static bool is_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return false;
}

static void navigate_to_directory(const char *path)
{
    strcpy(fm_state.current_path, path);
    refresh_file_list();
}

static void show_file_info(const char *filepath)
{
    struct stat st;
    if (stat(filepath, &st) != 0) {
        lv_label_set_text(fm_state.status_label, "无法获取文件信息");
        return;
    }
    
    // Create a simple message box with file info
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mbox, "文件信息");
    
    char info_text[600];
    const char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;
    
    // Limit filename display length to prevent buffer overflow
    char display_name[256];
    if (strlen(filename) > 200) {
        snprintf(display_name, sizeof(display_name), "%.197s...", filename);
    } else {
        strcpy(display_name, filename);
    }
    
    snprintf(info_text, sizeof(info_text), 
        "名称: %s\n"
        "大小: %ld 字节\n"
        "类型: %s",
        display_name,
        st.st_size,
        S_ISDIR(st.st_mode) ? "文件夹" : "文件"
    );
    
    lv_msgbox_add_text(mbox, info_text);
    
    // Apply theme styling to the message box title
    lv_obj_t *title = lv_msgbox_get_title(mbox);
    if (title) {
        theme_apply_label_style(title);
    }
    
    // Apply theme styling to the message box content
    lv_obj_t *content = lv_msgbox_get_content(mbox);
    if (content) {
        // Find and style the text label within the content
        uint32_t child_count = lv_obj_get_child_count(content);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(content, i);
            if (lv_obj_check_type(child, &lv_label_class)) {
                theme_apply_label_style(child);
            }
        }
    }
    
    lv_msgbox_add_close_button(mbox);
    
    // Apply theme styling to buttons in the message box
    lv_obj_t *footer = lv_msgbox_get_footer(mbox);
    if (footer) {
        uint32_t child_count = lv_obj_get_child_count(footer);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(footer, i);
            if (lv_obj_check_type(child, &lv_button_class)) {
                theme_apply_button_style(child);
                // Override size for close button
                lv_obj_set_size(child, 80, 40);
            }
        }
    }
    
    // Center the message box
    lv_obj_center(mbox);
}

// App definition
const app_t APP_FILE_MANAGER = {
    .id = "file_manager",
    .name = "文件管理器",
    .launch = file_manager_launch
};