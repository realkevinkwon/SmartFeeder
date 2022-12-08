#ifndef __FOOD_DISPENSER_H__
#define __FOOD_DISPENSER_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "stepper.h"
#include "load_cell.h"

#define FOOD_LC_init() LC1_init();
#define FOOD_LC_to_grams(x) LC1_to_grams(x);

// All weights are in grams
#define FOOD_BOWL_EMPTY_WEIGHT 37.0
#define FOOD_BOWL_FULL_WEIGHT 104.0

#define MIN_FOOD_DISPENSED 0.0
#define FOOD_PER_MS 10.0

#define FOOD_DISPENSER_CLOSED_ANGLE 0.0
#define FOOD_DISPENSER_OPEN_ANGLE 90.0

#define dispense_food(amount) fill_food_to_amount(FOOD_BOWL_FULL_WEIGHT - amount)

#ifdef __cplusplus
extern "C" {
#endif

void food_dispenser_init();
float get_food_level();
void fill_food_to_amount(float amount);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __FOOD_DISPENSER_H__ */