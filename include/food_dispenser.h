#ifndef __FOOD_DISPENSER_H__
#define __FOOD_DISPENSER_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "stepper.h"
#include "loadcell.h"

#define FOOD_LC_init() LC2_init();
#define FOOD_LC_calibration(load_cell) lc2_calibration(load_cell);
#define FOOD_LC_to_grams(x) lc2_to_grams(x);

// All weights are in grams
#define FOOD_BOWL_EMPTY_WEIGHT 0.0
#define FOOD_BOWL_MISSING_THRESHOLD FOOD_BOWL_EMPTY_WEIGHT-25.0
#define FOOD_BOWL_FULL_WEIGHT 50.0

#define FOOD_OPEN_TIME 100
#define MIN_FOOD_DISPENSED 5.0
#define MIN_FOOD_DIFFERENCE 1.0
#define MAX_FOOD_DISPENSE_ATTEMPTS 4

#define FOOD_OPEN_DIRECTION 1
#define FOOD_CLOSE_DIRECTION !FOOD_OPEN_DIRECTION
#define FOOD_CLOSED_ANGLE 52.5
#define FOOD_OPEN_ANGLE 45.0

#ifdef __cplusplus
extern "C" {
#endif

void food_dispenser_init();
float get_food_level();
void _dispense_food();
void fill_food_to_amount(float amount);
void dispense_food_amount(float amount);
void food_dispenser_calibration();

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __FOOD_DISPENSER_H__ */