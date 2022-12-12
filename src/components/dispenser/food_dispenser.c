#include "food_dispenser.h"

// Tag for logging
static const char *FOOD_TAG = "Food Dispenser";

hx711_t food_load_cell;
// float prev_food_level = 0;
// float food_consumed = 0;

void food_dispenser_init() {
    stepper_init();
    food_load_cell = FOOD_LC_init();
    FOOD_LC_calibration(food_load_cell);
    // prev_food_level = get_food_level();
}

float get_food_level() {
    return FOOD_LC_to_grams(load_measure(food_load_cell));
}

void _dispense_food() {
    step_angle(FOOD_OPEN_ANGLE, FOOD_OPEN_DIRECTION);
    vTaskDelay(FOOD_OPEN_TIME / portTICK_PERIOD_MS);
    step_angle(FOOD_CLOSED_ANGLE, FOOD_CLOSE_DIRECTION);
}

void fill_food_to_amount(float amount) {
    // Ensure we do not overfill the bowl
    float target_level = amount < FOOD_BOWL_FULL_WEIGHT ? amount : FOOD_BOWL_FULL_WEIGHT;
    float food_level = get_food_level();

    // Ensure that the bowl is present
    if (food_level <= FOOD_BOWL_MISSING_THRESHOLD) {
        ESP_LOGI(FOOD_TAG, "Bowl Missing!");
        return;
    }

    // Check if the food level is already at the target
    if ((target_level - food_level) >= MIN_FOOD_DISPENSED) {
        ESP_LOGI(FOOD_TAG, "Filling to %fg", target_level);

        // Start dispensing food
        int32_t attempts = 0;
        float last_food_level;
        while((target_level - food_level) >= MIN_FOOD_DISPENSED) {
            last_food_level = food_level;
            _dispense_food();
            food_level = get_food_level();

            // Check that the bowl is still present
            if (food_level <= FOOD_BOWL_MISSING_THRESHOLD) {
                ESP_LOGI(FOOD_TAG, "Bowl Missing!");
                break;
            }

            // Check that food is still being dispensed (ie: the reservoir is at a sufficient level)
            if (food_level - last_food_level < MIN_FOOD_DIFFERENCE) {
                attempts++;
                if (attempts > MAX_FOOD_DISPENSE_ATTEMPTS) {
                    ESP_LOGI(FOOD_TAG, "Reservoir Empty!");
                    break;
                }
            } else {
                attempts = 0;
            }

            // Wait before checking again
            vTaskDelay(FOOD_OPEN_TIME / portTICK_PERIOD_MS);
        }
    }
    ESP_LOGI(FOOD_TAG, "Food at %fg", food_level);
}

void dispense_food_amount(float amount) {
    fill_food_to_amount(get_food_level() + amount);
}

void food_dispenser_calibration() {
    float initial_level = get_food_level();
    _dispense_food();
    float final_level = get_food_level();
    ESP_LOGI(FOOD_TAG, "Amount Dispensed: %fg", final_level - initial_level);
}