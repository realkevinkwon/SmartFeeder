#include <esp_log.h>
#include "driver/gpio.h"
#include "hx711.h"
#include "memory.h"
#include "loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static hx711_t dev = {
    .dout = GPIO_NUM_18,
    .pd_sck = GPIO_NUM_19,
    .gain = HX711_GAIN_A_64
};

static const char* TAG = "HX711";

void load_cell_init(void) {
    ESP_ERROR_CHECK(hx711_init(&dev));
}

void load_cell_write(void) {
    while (1) {
        esp_err_t r = hx711_wait(&dev, 500);
        if (r != ESP_OK) {
            ESP_LOGE(TAG, "Device not found: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }

        int32_t data;
        r = hx711_read_average(&dev, 5, &data);
        if (r != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read data: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void load_cell_read(void) {

}