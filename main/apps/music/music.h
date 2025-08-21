#pragma once
#include "managers/app_manager.h"
#include "managers/window_manager.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simplified MP3 file info (removed metadata fields)
typedef struct {
    char filename[256];     // Full file path
    char title[128];        // Simple title from filename
} mp3_file_info_t;

// Simple play states
typedef enum {
    PLAY_STATE_STOPPED,
    PLAY_STATE_PLAYING,
    PLAY_STATE_PAUSED
} play_state_t;

// Simplified music player data (removed complex tracking)
typedef struct {
    mp3_file_info_t* files;
    uint32_t file_count;
    uint32_t current_index;
    bool is_scanning;
    bool sd_card_mounted;
    play_state_t play_state;
    wm_window_t* window;    // Window reference for windowed mode
} music_player_data_t;

// Core functions
extern const app_t APP_MUSIC;
void music_scan_files(music_player_data_t* data);
void music_free_files(music_player_data_t* data);
bool music_is_mp3_file(const char* filename);
void music_extract_title(const char* filename, char* title, size_t title_size);

// Playback control (simplified)
bool music_play_current(music_player_data_t* data);
void music_pause(music_player_data_t* data);
void music_resume(music_player_data_t* data);
void music_stop(music_player_data_t* data);
void music_play_next(music_player_data_t* data);
void music_play_previous(music_player_data_t* data);

#ifdef __cplusplus
}
#endif