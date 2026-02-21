/**
 * LVGL Dashboard Implementation (C++)
 * Target Resolution: 800x480
 * Framework: LVGL v8.3+
 */

#include "lvgl.h"

/* State Variables */
static float target_temp = 22.0f;
static float current_temp = 21.4f;
static int brightness1 = 75;
static int brightness2 = 30;
static bool is_dark_mode = true;

/* UI Objects */
static lv_obj_t * main_cont;
static lv_obj_t * sidebar;
static lv_obj_t * content_area;
static lv_obj_t * temp_label;
static lv_style_t style_card;

/* Event Handlers */
static void temp_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    int diff = (int)lv_event_get_user_data(e);
    if(code == LV_EVENT_CLICKED) {
        target_temp += (diff * 0.5f);
        lv_label_set_text_fmt(temp_label, "%.1f°C", target_temp);
    }
}

static void theme_toggle_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        is_dark_mode = !is_dark_mode;
        if(is_dark_mode) {
            lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_EMERALD), lv_palette_main(LV_PALETTE_GREY), true, LV_FONT_DEFAULT);
        } else {
            lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_EMERALD), lv_palette_main(LV_PALETTE_GREY), false, LV_FONT_DEFAULT);
        }
    }
}

void create_battery_indicator(lv_obj_t * parent, int soc) {
    lv_obj_t * bat_cont = lv_obj_create(parent);
    lv_obj_set_size(bat_cont, 60, 30);
    lv_obj_set_style_border_width(bat_cont, 2, 0);
    lv_obj_set_style_radius(bat_cont, 4, 0);
    lv_obj_clear_flag(bat_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bar = lv_bar_create(bat_cont);
    lv_obj_set_size(bar, 45, 20);
    lv_obj_align(bar, LV_ALIGN_LEFT_MID, 0, 0);
    lv_bar_set_value(bar, soc, LV_ANIM_OFF);
    
    if(soc > 50) lv_obj_set_style_bg_color(bar, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
    else if(soc > 20) lv_obj_set_style_bg_color(bar, lv_palette_main(LV_PALETTE_AMBER), LV_PART_INDICATOR);
    else lv_obj_set_style_bg_color(bar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);

    lv_obj_t * tip = lv_obj_create(bat_cont);
    lv_obj_set_size(tip, 4, 12);
    lv_obj_align(tip, LV_ALIGN_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_color(tip, lv_color_white(), 0);
}

void setup_dashboard(void) {
    /* Initialize Styles */
    lv_style_init(&style_card);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, lv_palette_lighten(LV_PALETTE_GREY, 2));

    /* Main Container */
    main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 800, 480);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(main_cont, 0, 0);

    /* Sidebar */
    sidebar = lv_obj_create(main_cont);
    lv_obj_set_size(sidebar, 160, 480);
    lv_obj_align(sidebar, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_radius(sidebar, 0, 0);
    lv_obj_set_style_bg_color(sidebar, lv_palette_darken(LV_PALETTE_GREY, 4), 0);

    lv_obj_t * logo = lv_label_create(sidebar);
    lv_label_set_text(logo, "CONTROL");
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_20, 0);
    lv_obj_align(logo, LV_ALIGN_TOP_LEFT, 10, 20);

    const char * menu_items[] = {"Home", "Air heater", "BMS", "Tanks", "Settings"};
    for(int i = 0; i < 5; i++) {
        lv_obj_t * btn = lv_btn_create(sidebar);
        lv_obj_set_size(btn, 140, 40);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 80 + (i * 50));
        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, menu_items[i]);
        lv_obj_center(lbl);
        if(i == 0) lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_EMERALD), 0);
    }

    /* Content Area */
    content_area = lv_obj_create(main_cont);
    lv_obj_set_size(content_area, 640, 480);
    lv_obj_align(content_area, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_pad_all(content_area, 15, 0);

    /* Top Row: Battery */
    create_battery_indicator(content_area, 84);

    /* Gauges (Meters) */
    lv_obj_t * meter_amp = lv_meter_create(content_area);
    lv_obj_set_size(meter_amp, 180, 180);
    lv_obj_align(meter_amp, LV_ALIGN_TOP_LEFT, 0, 50);
    lv_meter_scale_t * scale = lv_meter_add_scale(meter_amp);
    lv_meter_set_scale_ticks(meter_amp, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meter_amp, scale, 0, 50, 270, 135);

    /* Thermostat Card */
    lv_obj_t * thermo_card = lv_obj_create(content_area);
    lv_obj_set_size(thermo_card, 200, 150);
    lv_obj_align(thermo_card, LV_ALIGN_TOP_RIGHT, 0, 50);
    lv_obj_add_style(thermo_card, &style_card, 0);

    temp_label = lv_label_create(thermo_card);
    lv_label_set_text_fmt(temp_label, "%.1f°C", target_temp);
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_32, 0);
    lv_obj_align(temp_label, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t * btn_plus = lv_btn_create(thermo_card);
    lv_obj_set_size(btn_plus, 40, 40);
    lv_obj_align(btn_plus, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_t * lbl_plus = lv_label_create(btn_plus);
    lv_label_set_text(lbl_plus, "+");
    lv_obj_center(lbl_plus);
    lv_obj_add_event_cb(btn_plus, temp_btn_event_cb, LV_EVENT_CLICKED, (void*)1);

    /* Chart */
    lv_obj_t * chart = lv_chart_create(content_area);
    lv_obj_set_size(chart, 300, 150);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_EMERALD), LV_CHART_AXIS_PRIMARY_Y);
    for(int i = 0; i < 10; i++) lv_chart_set_next_value(chart, ser, lv_rand(10, 40));

    /* Sliders */
    lv_obj_t * slider1 = lv_slider_create(content_area);
    lv_obj_set_size(slider1, 200, 10);
    lv_obj_align(slider1, LV_ALIGN_BOTTOM_RIGHT, 0, -50);
    lv_slider_set_value(slider1, brightness1, LV_ANIM_OFF);

    lv_obj_t * slider2 = lv_slider_create(content_area);
    lv_obj_set_size(slider2, 200, 10);
    lv_obj_align(slider2, LV_ALIGN_BOTTOM_RIGHT, 0, -20);
    lv_slider_set_value(slider2, brightness2, LV_ANIM_OFF);
}
