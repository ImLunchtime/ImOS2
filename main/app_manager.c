#include "app_manager.h"

#ifndef APP_MANAGER_MAX
#define APP_MANAGER_MAX 16
#endif

static const app_t* s_apps[APP_MANAGER_MAX];
static size_t s_app_count = 0;

void app_manager_init(void)
{
    s_app_count = 0;
}

int app_manager_register(const app_t *app)
{
    if(!app) return -1;
    if(s_app_count >= APP_MANAGER_MAX) return -1;
    s_apps[s_app_count++] = app;
    return 0;
}

const app_t** app_manager_list(size_t *count)
{
    if(count) *count = s_app_count;
    return s_apps;
}