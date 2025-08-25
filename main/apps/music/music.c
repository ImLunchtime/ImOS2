#include "apps/music/music.h"
#include "managers/window_manager.h"
#include "hals/hal_audio.h"
#include "hals/hal_sdcard.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Global music player data
static music_player_data_t g_music_data = {
    .files = NULL,
    .file_count = 0,
    .current_index = 0,
    .is_scanning = false,
    .sd_card_mounted = false,
    .play_state = PLAY_STATE_STOPPED,
    .window = NULL
};

// UI element references
static lv_obj_t* g_file_list = NULL;
static lv_obj_t* g_current_song_label = NULL;
static lv_obj_t* g_play_pause_btn = NULL;
static lv_obj_t* g_prev_btn = NULL;
static lv_obj_t* g_next_btn = NULL;

// Forward declarations
static void create_music_ui(lv_obj_t* parent);
static void refresh_file_list(void);
static void update_current_song_display(void);
static void file_list_event_cb(lv_event_t* e);
static void play_pause_event_cb(lv_event_t* e);
static void prev_event_cb(lv_event_t* e);
static void next_event_cb(lv_event_t* e);

// Simplified MP3 file check
bool music_is_mp3_file(const char* filename) {
    if (!filename) return false;
    
    size_t len = strlen(filename);
    if (len < 4) return false;
    
    const char* ext = filename + len - 4;
    return (strcasecmp(ext, ".mp3") == 0);
}

// Extract simple title from filename
void music_extract_title(const char* filename, char* title, size_t title_size) {
    if (!filename || !title || title_size == 0) return;
    
    const char* basename = strrchr(filename, '/');
    if (basename) {
        basename++;
    } else {
        basename = filename;
    }
    
    strncpy(title, basename, title_size - 1);
    title[title_size - 1] = '\0';
    
    // Remove .mp3 extension
    char* dot = strrchr(title, '.');
    if (dot && strcasecmp(dot, ".mp3") == 0) {
        *dot = '\0';
    }
}

// Scan for MP3 files (simplified)
void music_scan_files(music_player_data_t* data) {
    if (!data) return;
    
    // Free existing files
    music_free_files(data);
    
    // Check SD card
    data->sd_card_mounted = hal_sdcard_is_mounted();
    if (!data->sd_card_mounted) {
        printf("SD card not mounted\n");
        return;
    }
    
    data->is_scanning = true;
    
    const char* mount_point = hal_sdcard_get_mount_point();
    DIR* dir = opendir(mount_point);
    if (!dir) {
        printf("Failed to open SD card directory\n");
        data->is_scanning = false;
        return;
    }
    
    // Count MP3 files
    struct dirent* entry;
    uint32_t mp3_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && music_is_mp3_file(entry->d_name)) {
            mp3_count++;
        }
    }
    
    if (mp3_count == 0) {
        printf("No MP3 files found\n");
        closedir(dir);
        data->is_scanning = false;
        return;
    }
    
    // Allocate memory
    data->files = (mp3_file_info_t*)malloc(mp3_count * sizeof(mp3_file_info_t));
    if (!data->files) {
        printf("Failed to allocate memory for MP3 files\n");
        closedir(dir);
        data->is_scanning = false;
        return;
    }
    
    // Scan again and store files
    rewinddir(dir);
    data->file_count = 0;
    
    while ((entry = readdir(dir)) != NULL && data->file_count < mp3_count) {
        if (entry->d_type == DT_REG && music_is_mp3_file(entry->d_name)) {
            // Store full path
            snprintf(data->files[data->file_count].filename, 
                    sizeof(data->files[data->file_count].filename),
                    "%s/%s", mount_point, entry->d_name);
            
            // Extract title
            music_extract_title(entry->d_name, 
                              data->files[data->file_count].title,
                              sizeof(data->files[data->file_count].title));
            
            data->file_count++;
        }
    }
    
    closedir(dir);
    data->is_scanning = false;
    
    printf("Found %lu MP3 files\n", (unsigned long)data->file_count);
}

// Free MP3 files
void music_free_files(music_player_data_t* data) {
    if (data && data->files) {
        free(data->files);
        data->files = NULL;
        data->file_count = 0;
        data->current_index = 0;
    }
}

// Playback functions (simplified)
bool music_play_current(music_player_data_t* data) {
    if (!data || !data->files || data->current_index >= data->file_count) {
        return false;
    }
    
    // Stop any current playback first to prevent conflicts
    if (data->play_state == PLAY_STATE_PLAYING || data->play_state == PLAY_STATE_PAUSED) {
        hal_audio_stop_mp3();
    }
    
    const char* file_path = data->files[data->current_index].filename;
    printf("Playing: %s\n", file_path);
    
    if (hal_audio_play_mp3_file(file_path)) {
        data->play_state = PLAY_STATE_PLAYING;
        update_current_song_display();
        return true;
    }
    
    return false;
}

void music_pause(music_player_data_t* data) {
    if (data && data->play_state == PLAY_STATE_PLAYING) {
        hal_audio_stop_mp3();
        data->play_state = PLAY_STATE_PAUSED;
        update_current_song_display();
    }
}

void music_resume(music_player_data_t* data) {
    if (data && data->play_state == PLAY_STATE_PAUSED) {
        music_play_current(data);
    }
}

void music_stop(music_player_data_t* data) {
    if (data) {
        hal_audio_stop_mp3();
        data->play_state = PLAY_STATE_STOPPED;
        update_current_song_display();
    }
}

void music_play_next(music_player_data_t* data) {
    if (!data || !data->files || data->file_count == 0) return;
    
    data->current_index = (data->current_index + 1) % data->file_count;
    music_play_current(data);
}

void music_play_previous(music_player_data_t* data) {
    if (!data || !data->files || data->file_count == 0) return;
    
    if (data->current_index == 0) {
        data->current_index = data->file_count - 1;
    } else {
        data->current_index--;
    }
    music_play_current(data);
}

// UI Functions
static void update_current_song_display(void) {
    if (!g_current_song_label) return;
    
    if (g_music_data.files && g_music_data.current_index < g_music_data.file_count) {
        const char* title = g_music_data.files[g_music_data.current_index].title;
        const char* state_text = "";
        
        switch (g_music_data.play_state) {
            case PLAY_STATE_PLAYING: state_text = " ♪"; break;
            case PLAY_STATE_PAUSED: state_text = " ⏸"; break;
            default: state_text = ""; break;
        }
        
        char display_text[160];
        snprintf(display_text, sizeof(display_text), "%s%s", title, state_text);
        lv_label_set_text(g_current_song_label, display_text);
    } else {
        lv_label_set_text(g_current_song_label, "No song selected");
    }
    
    // Update play/pause button
    if (g_play_pause_btn) {
        lv_obj_t* label = lv_obj_get_child(g_play_pause_btn, 0);
        if (label) {
            const char* symbol = (g_music_data.play_state == PLAY_STATE_PLAYING) ? 
                                LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY;
            lv_label_set_text(label, symbol);
        }
    }
}

static void refresh_file_list(void) {
    if (!g_file_list) return;
    
    lv_obj_clean(g_file_list);
    
    if (!g_music_data.files || g_music_data.file_count == 0) {
        lv_obj_t* item = lv_list_add_text(g_file_list, "No MP3 files found");
        lv_obj_set_style_text_color(item, lv_palette_main(LV_PALETTE_GREY), 0);
        return;
    }
    
    for (uint32_t i = 0; i < g_music_data.file_count; i++) {
        lv_obj_t* item = lv_list_add_btn(g_file_list, LV_SYMBOL_AUDIO, g_music_data.files[i].title);
        lv_obj_add_event_cb(item, file_list_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
        
        // Highlight current song
        if (i == g_music_data.current_index) {
            lv_obj_set_style_bg_color(item, lv_palette_main(LV_PALETTE_BLUE), 0);
            lv_obj_set_style_bg_opa(item, LV_OPA_30, 0);
        }
    }
}

// Event handlers
static void file_list_event_cb(lv_event_t* e) {
    uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    
    if (index < g_music_data.file_count) {
        g_music_data.current_index = index;
        music_play_current(&g_music_data);
        refresh_file_list(); // Refresh to update highlighting
    }
}

static void play_pause_event_cb(lv_event_t* e) {
    if (g_music_data.play_state == PLAY_STATE_PLAYING) {
        music_pause(&g_music_data);
    } else {
        if (g_music_data.play_state == PLAY_STATE_PAUSED) {
            music_resume(&g_music_data);
        } else {
            music_play_current(&g_music_data);
        }
    }
}

static void prev_event_cb(lv_event_t* e) {
    music_play_previous(&g_music_data);
    refresh_file_list();
}

static void next_event_cb(lv_event_t* e) {
    music_play_next(&g_music_data);
    refresh_file_list();
}

// Create simplified windowed UI
static void create_music_ui(lv_obj_t* parent) {
    // Main container with flex layout
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_gap(parent, 10, 0);
    
    // Current song display
    g_current_song_label = lv_label_create(parent);
    lv_label_set_text(g_current_song_label, "No song selected");
    lv_obj_set_style_text_align(g_current_song_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(g_current_song_label, LV_PCT(100));
    lv_label_set_long_mode(g_current_song_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    
    // Control buttons container
    lv_obj_t* btn_container = lv_obj_create(parent);
    lv_obj_set_size(btn_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_layout(btn_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(btn_container, 5, 0);
    lv_obj_set_style_pad_gap(btn_container, 10, 0);
    
    // Previous button
    g_prev_btn = lv_btn_create(btn_container);
    lv_obj_set_size(g_prev_btn, 50, 50);
    lv_obj_add_event_cb(g_prev_btn, prev_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* prev_label = lv_label_create(g_prev_btn);
    lv_label_set_text(prev_label, LV_SYMBOL_PREV);
    lv_obj_center(prev_label);
    
    // Play/Pause button
    g_play_pause_btn = lv_btn_create(btn_container);
    lv_obj_set_size(g_play_pause_btn, 60, 60);
    lv_obj_add_event_cb(g_play_pause_btn, play_pause_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* play_label = lv_label_create(g_play_pause_btn);
    lv_label_set_text(play_label, LV_SYMBOL_PLAY);
    lv_obj_center(play_label);
    
    // Next button
    g_next_btn = lv_btn_create(btn_container);
    lv_obj_set_size(g_next_btn, 50, 50);
    lv_obj_add_event_cb(g_next_btn, next_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* next_label = lv_label_create(g_next_btn);
    lv_label_set_text(next_label, LV_SYMBOL_NEXT);
    lv_obj_center(next_label);
    
    // File list
    g_file_list = lv_list_create(parent);
    lv_obj_set_size(g_file_list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(g_file_list, 1);
}

// Main launch function
static void music_launch(void) {
    // Create window
    g_music_data.window = wm_open_window("Music Player", true, LV_PCT(70), LV_PCT(60));
    lv_obj_t* content = wm_get_content(g_music_data.window);
    
    // Create UI
    create_music_ui(content);
    
    // Initialize audio HAL
    hal_audio_init();
    
    // Scan for MP3 files
    music_scan_files(&g_music_data);
    refresh_file_list();
    update_current_song_display();
}

// App definition
const app_t APP_MUSIC = {
    .id = "music",
    .name = "Music",
    .launch = music_launch
};