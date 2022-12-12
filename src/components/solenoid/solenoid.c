#include "solenoid.h"

// Tag for logging
static const char *SOLENOID_TAG = "Solenoid";

void solenoid_init() {
    // Configure the gpio pin as output
    gpio_reset_pin(SOLENOID_PIN);
    gpio_set_direction(SOLENOID_PIN, GPIO_MODE_OUTPUT);
    close_solenoid();
    ESP_LOGI(SOLENOID_TAG, "Solenoid Closed");
}

void solenoid_test() {
    solenoid_init();
    while (1) {
        open_solenoid();
        ESP_LOGI(SOLENOID_TAG, "Solenoid Open");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        close_solenoid();
        ESP_LOGI(SOLENOID_TAG, "Solenoid Closed");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }   
}