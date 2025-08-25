#include "os.h"
#include "managers/app_manager.h"
#include "apps/launcher/launcher.h"
#include "apps/settings/settings.h"
#include "apps/music/music.h"
#include "control_center/control_center.h"

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
    
    // Initialize control center system AFTER launcher to ensure it's on top
    control_center_init();
}