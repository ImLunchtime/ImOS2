#include "os.h"
#include "app_manager.h"
#include "launcher.h"
#include "settings.h"
#include "music.h"

void os_init(lv_disp_t *disp)
{
    LV_UNUSED(disp);

    // Initialize app manager and register apps (Launcher is system-managed)
    app_manager_init();

    // Register user apps shown in Launcher
    app_manager_register(&APP_SETTINGS);
    app_manager_register(&APP_MUSIC);

    // Start launcher (non-closable, always present underneath)
    launcher_open();
}