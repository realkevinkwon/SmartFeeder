#include "food_dispenser.h"

// Tag for logging
// static const char *TAG = "Food Dispenser";

hx711_t load_cell;

void food_dispenser_init() {
    stepper_init();
    load_cell = FOOD_LC_init();
}

float get_food_level() {
    return FOOD_LC_to_grams(load_measure(load_cell));
}

void fill_food_to_amount(float amount) {
    float target_amount = amount < FOOD_BOWL_FULL_WEIGHT ? amount : FOOD_BOWL_FULL_WEIGHT;
    float amount_to_dispense = target_amount - get_food_level();

    // If the amount to dispense is less than the minimum food that can be dispensed at one time then return
    if (amount_to_dispense < MIN_FOOD_DISPENSED) {return;}

    // Determine how long the food dispenser needs to be open
    float open_time = (amount_to_dispense - MIN_FOOD_DISPENSED) / FOOD_PER_MS;

    // Open the food dispenser for the appropriate amount of time
    step_to_angle(FOOD_DISPENSER_OPEN_ANGLE);
    vTaskDelay(open_time / portTICK_PERIOD_MS);
    step_to_angle(FOOD_DISPENSER_CLOSED_ANGLE);
}