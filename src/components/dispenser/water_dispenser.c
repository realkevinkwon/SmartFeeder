#include "water_dispenser.h"

// Tag for logging
static const char *WATER_TAG = "Water Dispenser";

hx711_t water_load_cell;
// float prev_water_level = 0;
// float water_consumed = 0;

void water_dispenser_init() {
    solenoid_init();
    water_load_cell = WATER_LC_init();
    WATER_LC_calibration(water_load_cell);
    // prev_water_level = get_water_level();
}

float get_water_level() {
    return WATER_LC_to_grams(load_measure(water_load_cell));
}

void fill_water_to_amount(float amount) {
    // Ensure we do not overfill the bowl
    float target_level = amount < WATER_BOWL_FULL_WEIGHT ? amount : WATER_BOWL_FULL_WEIGHT;
    float water_level = get_water_level();

    // Ensure that the bowl is present
    if (water_level <= WATER_BOWL_MISSING_THRESHOLD) {
        ESP_LOGI(WATER_TAG, "Bowl Missing!");
        return;
    }

    // Check if the water level is already at the target
    if ((target_level - water_level) >= MIN_WATER_DISPENSED) {
        ESP_LOGI(WATER_TAG, "Filling to %fg", target_level);

        // Start dispensing water
        int32_t attempts = 0;
        float last_water_level;
        open_solenoid();
        while((target_level - water_level) >= MIN_WATER_DISPENSED) {
            last_water_level = water_level;
            water_level = get_water_level();

            // Check that the bowl is still present
            if (water_level <= WATER_BOWL_MISSING_THRESHOLD) {
                ESP_LOGI(WATER_TAG, "Bowl Missing!");
                break;
            }

            // Check that water is still being dispensed (ie: the reservoir is at a sufficient level)
            if (water_level - last_water_level < MIN_WATER_DIFFERENCE) {
                attempts++;
                if (attempts > MAX_WATER_DISPENSE_ATTEMPTS) {
                    ESP_LOGI(WATER_TAG, "Reservoir Empty!");
                    break;
                }
            } else {
                attempts = 0;
            }

            // Wait before checking again
            vTaskDelay(WATER_OPEN_TIME / portTICK_PERIOD_MS);
        }
        // Stop dispensing water
        close_solenoid();
    }
    ESP_LOGI(WATER_TAG, "Water at %fg", water_level);
}

void dispense_water_amount(float amount) {
    fill_water_to_amount(get_water_level() + amount);
}

void water_dispenser_calibration() {
    float initial_level = get_water_level();
    open_solenoid();
    vTaskDelay(WATER_OPEN_TIME / portTICK_PERIOD_MS);
    close_solenoid();
    float final_level = get_water_level();
    ESP_LOGI(WATER_TAG, "Amount Dispensed: %fg", final_level - initial_level);
}