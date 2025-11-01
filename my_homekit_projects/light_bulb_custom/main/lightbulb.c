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
#include "driver/ledc.h"

static const char *TAG = "lightbulb";

#define LEDC_IO25_TIMER              LEDC_TIMER_0
#define LEDC_IO25_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_IO25_OUTPUT_IO          (25) // Define the output GPIO
#define LEDC_IO25_CHANNEL            LEDC_CHANNEL_0
#define LEDC_IO25_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_IO25_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

#define LEDC_IO26_TIMER              LEDC_TIMER_0
#define LEDC_IO26_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_IO26_OUTPUT_IO          (26) // Define the output GPIO
#define LEDC_IO26_CHANNEL            LEDC_CHANNEL_1
#define LEDC_IO26_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_IO26_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

#define LEDC_IO27_TIMER              LEDC_TIMER_0
#define LEDC_IO27_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_IO27_OUTPUT_IO          (27) // Define the output GPIO
#define LEDC_IO27_CHANNEL            LEDC_CHANNEL_2
#define LEDC_IO27_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_IO27_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

static uint8_t s_led_state = 0;
// static uint32_t dimming_value = 0;

static float s_hue = 0;
static float s_saturation = 0;
static int s_brightness = 100;

// Convert Hue [0-360], Saturation [0-100], Brightness [0-100] to RGB (0–8191)
static void hsb_to_rgb(float hue, float sat, float bri, uint32_t *r, uint32_t *g, uint32_t *b) {
    float hh, p, q, t, ff;
    long i;
    float r_f, g_f, b_f;

    sat /= 100.0;
    bri /= 100.0;

    if (sat <= 0.0) {
        r_f = g_f = b_f = bri;
    } else {
        hh = hue;
        if (hh >= 360.0) hh = 0.0;
        hh /= 60.0;
        i = (long)hh;
        ff = hh - i;
        p = bri * (1.0 - sat);
        q = bri * (1.0 - (sat * ff));
        t = bri * (1.0 - (sat * (1.0 - ff)));

        switch (i) {
            case 0:  r_f = bri; g_f = t; b_f = p; break;
            case 1:  r_f = q; g_f = bri; b_f = p; break;
            case 2:  r_f = p; g_f = bri; b_f = t; break;
            case 3:  r_f = p; g_f = q; b_f = bri; break;
            case 4:  r_f = t; g_f = p; b_f = bri; break;
            case 5:
            default: r_f = bri; g_f = p; b_f = q; break;
        }
    }

    // Convert 0–1 float to LEDC duty range (13-bit = 0–8191)
    *r = (uint32_t)(r_f * 8191);
    *g = (uint32_t)(g_f * 8191);
    *b = (uint32_t)(b_f * 8191);
}

static void update_led_color(void) 
{
    uint32_t r, g, b;
    hsb_to_rgb(s_hue, s_saturation, s_brightness, &r, &g, &b);

    ESP_LOGI(TAG, "RGB -> R:%d  G:%d  B:%d", r, g, b);

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO25_MODE, LEDC_IO25_CHANNEL, r));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO25_MODE, LEDC_IO25_CHANNEL));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO26_MODE, LEDC_IO26_CHANNEL, g));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO26_MODE, LEDC_IO26_CHANNEL));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO27_MODE, LEDC_IO27_CHANNEL, b));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO27_MODE, LEDC_IO27_CHANNEL));
}

/**
 * @brief initialize the lightbulb lowlevel module
 */
void lightbulb_init(void)
{
    ESP_LOGI(TAG, "Dummy Light Driver Init.");

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_IO25_timer = {
        .speed_mode       = LEDC_IO25_MODE,
        .duty_resolution  = LEDC_IO25_DUTY_RES,
        .timer_num        = LEDC_IO25_TIMER,
        .freq_hz          = LEDC_IO25_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_IO25_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_IO25_channel = {
        .speed_mode     = LEDC_IO25_MODE,
        .channel        = LEDC_IO25_CHANNEL,
        .timer_sel      = LEDC_IO25_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_IO25_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_IO25_channel));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_IO26_timer = {
        .speed_mode       = LEDC_IO26_MODE,
        .duty_resolution  = LEDC_IO26_DUTY_RES,
        .timer_num        = LEDC_IO26_TIMER,
        .freq_hz          = LEDC_IO26_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_IO26_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_IO26_channel = {
        .speed_mode     = LEDC_IO26_MODE,
        .channel        = LEDC_IO26_CHANNEL,
        .timer_sel      = LEDC_IO26_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_IO26_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_IO26_channel));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_IO27_timer = {
        .speed_mode       = LEDC_IO27_MODE,
        .duty_resolution  = LEDC_IO27_DUTY_RES,
        .timer_num        = LEDC_IO27_TIMER,
        .freq_hz          = LEDC_IO27_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_IO27_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_IO27_channel = {
        .speed_mode     = LEDC_IO27_MODE,
        .channel        = LEDC_IO27_CHANNEL,
        .timer_sel      = LEDC_IO27_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_IO27_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_IO27_channel));
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
        update_led_color();
    }
    else
    {
        s_led_state = 0;
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO25_MODE, LEDC_IO25_CHANNEL,  0 ) );
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO25_MODE, LEDC_IO25_CHANNEL));

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO26_MODE, LEDC_IO26_CHANNEL,  0 ) );
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO26_MODE, LEDC_IO26_CHANNEL));

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO27_MODE, LEDC_IO27_CHANNEL,  0) );
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO27_MODE, LEDC_IO27_CHANNEL));
    }
    

    return 0;
}

/**
 * @brief set the saturation of the lowlevel lightbulb
 */
int lightbulb_set_saturation(float value)
{
    ESP_LOGI(TAG, "lightbulb_set_saturation : %f", value);
    s_saturation = value;
    update_led_color();
    return 0;
}

/**
 * @brief set the hue of the lowlevel lightbulb
 */
int lightbulb_set_hue(float value)
{
    ESP_LOGI(TAG, "lightbulb_set_hue : %f", value);
    s_hue = value;
    update_led_color();
    return 0;
}

/**
 * @brief set the brightness of the lowlevel lightbulb
 */
int lightbulb_set_brightness(int value)
{
    // ESP_LOGI(TAG, "lightbulb_set_brightness : %d", value);
    // // Set duty to 50%
    // dimming_value = (( (value) * 8192 ) / 100);

    // ESP_LOGI(TAG, "dimming_value to led : %d", dimming_value);

    // ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO25_MODE, LEDC_IO25_CHANNEL,  dimming_value ) );
    // // Update duty to apply the new value
    // ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO25_MODE, LEDC_IO25_CHANNEL));

    // ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO26_MODE, LEDC_IO26_CHANNEL,  dimming_value ) );
    // // Update duty to apply the new value
    // ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO26_MODE, LEDC_IO26_CHANNEL));

    // ESP_ERROR_CHECK(ledc_set_duty(LEDC_IO27_MODE, LEDC_IO27_CHANNEL,  dimming_value ) );
    // // Update duty to apply the new value
    // ESP_ERROR_CHECK(ledc_update_duty(LEDC_IO27_MODE, LEDC_IO27_CHANNEL));
    ESP_LOGI(TAG, "lightbulb_set_brightness : %d", value);
    s_brightness = value;
    update_led_color();

    return 0;
}
