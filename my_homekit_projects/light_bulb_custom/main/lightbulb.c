/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS products only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* HomeKit Lightbulb Example Dummy Implementation
 * Refer ESP IDF docs for LED driver implementation:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html
*/

#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "lightbulb";

#define BLINK_GPIO_25 25
#define BLINK_GPIO_26 26
#define BLINK_GPIO_27 27

static uint8_t s_led_state = 0;

/**
 * @brief initialize the lightbulb lowlevel module
 */
void lightbulb_init(void)
{
    ESP_LOGI(TAG, "Dummy Light Driver Init.");
    gpio_reset_pin(BLINK_GPIO_25);
    gpio_reset_pin(BLINK_GPIO_26);
    gpio_reset_pin(BLINK_GPIO_27);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO_25, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_26, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_27, GPIO_MODE_OUTPUT);
}

/**
 * @brief turn on/off the lowlevel lightbulb
 */
int lightbulb_set_on(bool value)
{
    ESP_LOGI(TAG, "lightbulb_set_on : %s", value == true ? "true" : "false");
    if( value == true)
    {
        s_led_state = 1;
    }
    else
    {
        s_led_state = 0;
    }
    gpio_set_level(BLINK_GPIO_25, s_led_state);
    gpio_set_level(BLINK_GPIO_26, s_led_state);
    gpio_set_level(BLINK_GPIO_27, s_led_state);
    return 0;
}

/**
 * @brief set the saturation of the lowlevel lightbulb
 */
int lightbulb_set_saturation(float value)
{
    ESP_LOGI(TAG, "lightbulb_set_saturation : %f", value);
    return 0;
}

/**
 * @brief set the hue of the lowlevel lightbulb
 */
int lightbulb_set_hue(float value)
{
    ESP_LOGI(TAG, "lightbulb_set_hue : %f", value);
    return 0;
}

/**
 * @brief set the brightness of the lowlevel lightbulb
 */
int lightbulb_set_brightness(int value)
{
    ESP_LOGI(TAG, "lightbulb_set_brightness : %d", value);
    return 0;
}
