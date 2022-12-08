#include "loadcell.h"

// Tag for logging
static const char *TAG = "Load Cell";

hx711_t load_cell_init(gpio_num_t dout, gpio_num_t pd_sck) {
    hx711_t load_cell = {
        .dout = dout,
        .pd_sck = pd_sck,
        .gain = HX711_GAIN_A_64
    };
    ESP_ERROR_CHECK(hx711_init(&load_cell));
    return load_cell;
}

int32_t attempt_load_measure(hx711_t load_cell) {
    esp_err_t r = hx711_wait(&load_cell, 500);
    if (r != ESP_OK)
    {
        ESP_LOGE(TAG, "Device not found: %d (%s)\n", r, esp_err_to_name(r));
        return -1;
    }

    int32_t data;
    r = hx711_read_average(&load_cell, LC_NUM_MEASUREMENTS, &data);
    if (r != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read Data: %d (%s)\n", r, esp_err_to_name(r));
        return -1;
    }

    return data;
}

int32_t load_measure(hx711_t load_cell) {
    int32_t load_meas;
    for (int32_t i = 0; i < LC_NUM_ATTEMPTS; i++) {
        load_meas = attempt_load_measure(load_cell);
        if (load_meas != -1) {
            return load_meas;
        }
    }
    return -1;
}

void load_cell_test (void *pvParameters)
{
    hx711_t load_cell_1 = LC1_init();
    hx711_t load_cell_2 = LC2_init();

    while (1)
    {
        float lc1_meas = LC1_to_grams(load_measure(load_cell_1));
        float lc2_meas = LC2_to_grams(load_measure(load_cell_2));
        ESP_LOGI(TAG, "LC1: %fg   -   LC2: %fg", lc1_meas, lc2_meas);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}