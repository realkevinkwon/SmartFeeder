#ifndef __WATER_DISPENSER_H__
#define __WATER_DISPENSER_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "solenoid.h"
#include "loadcell.h"

#define WATER_LC_init() LC2_init();
#define WATER_LC_calibration(load_cell) lc2_calibration(load_cell);
#define WATER_LC_to_grams(x) lc2_to_grams(x);

// All weights are in grams
#define WATER_BOWL_EMPTY_WEIGHT 37.0
#define WATER_BOWL_FULL_WEIGHT 194.0

#define MIN_WATER_DISPENSED 0.0
#define WATER_PER_MS 10.0

#define dispense_water(amount) fill_water_to_amount(WATER_BOWL_FULL_WEIGHT - amount);

void water_dispenser_init();
float get_water_level();
void fill_water_to_amount(float amount);
void water_dispenser_calibration();

#endif /* __WATER_DISPENSER_H__ */