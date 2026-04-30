/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "bsp/esp-bsp.h"
#include "lvgl.h"

static const char *TAG = "app";
static lv_obj_t *title;

static void button_cb() {
    ESP_LOGI(TAG, "button clicked!");
}

uint16_t remap(uint16_t value, uint16_t  old_min, uint16_t  old_max,uint16_t  new_min, uint16_t new_max) {
    return (uint16_t) ((((value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min);
}

static void touch_event_cb(lv_event_t * e)
{
    lv_indev_t *indev = lv_indev_get_act();
    if (!indev) {
        return;
    }

    lv_point_t point;
    lv_indev_get_point(indev, &point);

    uint16_t remap_x = remap(point.x, 0, 320, 320, 0);
    ESP_LOGI(TAG, "touched: x: %d, y: %d", remap_x, point.y);
}

void app_main(void)
{
    ESP_LOGI(TAG, "reached main");
    bsp_display_start();
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);

    bsp_display_lock(0);
    title = lv_label_create(scr);
    lv_label_set_text(title, "Hello World!");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *button = lv_button_create(scr);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(button, button_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *button_label = lv_label_create(button);
    lv_label_set_text(button_label, "click me!");
    lv_obj_center(button_label);

    lv_indev_t *indev = bsp_display_get_input_dev();
    if (indev) {
        lv_indev_add_event_cb(indev, touch_event_cb, LV_EVENT_PRESSED, NULL);
    }

    bsp_display_unlock();
}
