#include "water_dispenser.h"

// Tag for logging
static const char *WATER_TAG = "Water Dispenser";

hx711_t load_cell;

void water_dispenser_init() {
    solenoid_init();
    load_cell = WATER_LC_init();
}

float get_water_level() {
    return WATER_LC_to_grams(load_measure(load_cell));
}

void fill_water_to_amount(float amount) {
    float target_amount = amount < WATER_BOWL_FULL_WEIGHT ? amount : WATER_BOWL_FULL_WEIGHT;
    float amount_to_dispense = target_amount - get_water_level();

    // If the amount to dispense is less than the minimum water that can be dispensed at one time then return
    if (amount_to_dispense < MIN_WATER_DISPENSED) {return;}

    // Determine how long the solenoid valve needs to be open
    float open_time = (amount_to_dispense - MIN_WATER_DISPENSED) / WATER_PER_MS;

    // Open the solenoid valve for the appropriate amount of time
    open_solenoid();
    vTaskDelay(open_time / portTICK_PERIOD_MS);
    close_solenoid();
}

void water_dispenser_calibration() {
    float initial_level = get_water_level();
    open_solenoid();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    close_solenoid();
    float final_level = get_water_level();
    ESP_LOGI(WATER_TAG, "Amount Dispensed: %fg", final_level - initial_level);
}