# Find all .c files in specific directories
file(GLOB_RECURSE APP_SRCS "apps/*.c")
file(GLOB_RECURSE HAL_SRCS "hals/*.c")
file(GLOB_RECURSE MANAGER_SRCS "managers/*.c")
file(GLOB_RECURSE CONTROL_SRCS "control_center/*.c")
file(GLOB_RECURSE ASSETS_SRCS "assets/*.c")

# Main source files
set(MAIN_SRCS
    "m5stack-tab5-lvgl.c"
    "gui.c"
    "os.c"
)

idf_component_register(SRCS ${MAIN_SRCS}
                            ${APP_SRCS}
                            ${HAL_SRCS}
                            ${MANAGER_SRCS}
                            ${CONTROL_SRCS}
                            ${ASSETS_SRCS}
                    INCLUDE_DIRS ".")