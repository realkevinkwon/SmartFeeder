#include "loadcell.h"

// Tag for logging
static const char *LOAD_CELL_TAG = "Load Cell";

int32_t lc1_cali_zero = 536624890;
int32_t lc2_cali_zero = 309132;

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
        ESP_LOGE(LOAD_CELL_TAG, "Device not found: %d (%s)\n", r, esp_err_to_name(r));
        return -1;
    }

    int32_t data;
    r = hx711_read_average(&load_cell, LC_NUM_MEASUREMENTS, &data);
    if (r != ESP_OK)
    {
        ESP_LOGE(LOAD_CELL_TAG, "Failed to read Data: %d (%s)\n", r, esp_err_to_name(r));
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

void lc1_calibration(hx711_t load_cell) {
    lc1_cali_zero = load_measure(load_cell);
}

float lc1_to_grams(int32_t raw) {
    return (raw-lc1_cali_zero)/LC1_CALI_ONE;
}

void lc2_calibration(hx711_t load_cell) {
    lc2_cali_zero = load_measure(load_cell);
}

float lc2_to_grams(int32_t raw) {
    return (raw-lc2_cali_zero)/LC2_CALI_ONE;
}

void load_cell_test()
{
    hx711_t load_cell_1 = LC1_init();
    lc1_calibration(load_cell_1);
    hx711_t load_cell_2 = LC2_init();
    lc2_calibration(load_cell_2);

    while (1)
    {
        int32_t lc1_meas = load_measure(load_cell_1);
        int32_t lc2_meas = load_measure(load_cell_2);
        ESP_LOGI(LOAD_CELL_TAG, "LC1: %d   -   %fg", lc1_meas, lc1_to_grams(lc1_meas));
        ESP_LOGI(LOAD_CELL_TAG, "LC2: %d   -   %fg", lc2_meas, lc2_to_grams(lc2_meas));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}