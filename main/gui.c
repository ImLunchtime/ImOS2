#include "gui.h"

void gui_init(lv_disp_t *disp) 
{
    // // Get the screen size
    // lv_coord_t screen_width = lv_display_get_horizontal_resolution(disp);
    // lv_coord_t screen_height = lv_display_get_vertical_resolution(disp);

    // // Create a button in the center
    // lv_obj_t *btn = lv_btn_create(lv_scr_act());
    
    // // Set button size (100x50 pixels)
    // lv_obj_set_size(btn, 100, 50);
    
    // // Center the button
    // lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    
    // // Add a label to the button
    // lv_obj_t *label = lv_label_create(btn);
    // lv_label_set_text(label, "Press Me!");
    // lv_obj_center(label);

    // // Make the button toggleable
    // lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    // lv_demo_widgets();
    // Initialize our simple OS
    #include "os.h"
    os_init(disp);
} 