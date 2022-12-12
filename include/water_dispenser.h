#ifndef __WATER_DISPENSER_H__
#define __WATER_DISPENSER_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "solenoid.h"
#include "loadcell.h"

#define WATER_LC_init() LC1_init();
#define WATER_LC_calibration(load_cell) lc1_calibration(load_cell);
#define WATER_LC_to_grams(x) lc1_to_grams(x);

// All weights are in grams
#define WATER_BOWL_EMPTY_WEIGHT 0.0
#define WATER_BOWL_MISSING_THRESHOLD WATER_BOWL_EMPTY_WEIGHT-5.0
#define WATER_BOWL_FULL_WEIGHT 150.0

#define WATER_OPEN_TIME 1000
#define MIN_WATER_DISPENSED 1.0
#define MIN_WATER_DIFFERENCE 0.2
#define MAX_WATER_DISPENSE_ATTEMPTS 4

#ifdef __cplusplus
extern "C" {
#endif

void water_dispenser_init();
float get_water_level();
void fill_water_to_amount(float amount);
void dispense_water_amount(float amount);
void water_dispenser_calibration();

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __WATER_DISPENSER_H__ */