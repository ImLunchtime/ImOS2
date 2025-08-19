#pragma once

#include <stddef.h>

typedef void (*app_launch_fn)(void);

typedef struct {
    const char *id;
    const char *name;
    app_launch_fn launch;
} app_t;

// Initialize registry
void app_manager_init(void);

// Register an app (returns 0 on success, -1 on full)
int app_manager_register(const app_t *app);

// Get the list of registered apps (returns pointer to internal array of pointers)
const app_t** app_manager_list(size_t *count);