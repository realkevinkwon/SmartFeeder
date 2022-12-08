#ifndef __WATER_DISPENSER_H__
#define __WATER_DISPENSER_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "solenoid.h"
#include "load_cell.h"

#define WATER_LC_init() LC2_init();
#define WATER_LC_to_grams(x) LC2_to_grams(x);

// All weights are in grams
#define WATER_BOWL_EMPTY_WEIGHT 37.0
#define WATER_BOWL_FULL_WEIGHT 194.0

#define MIN_WATER_DISPENSED 0.0
#define WATER_PER_MS 10.0

#define dispense_water(amount) fill_water_to_amount(WATER_BOWL_FULL_WEIGHT - amount)

#ifdef __cplusplus
extern "C" {
#endif

void water_dispenser_init();
float get_water_level();
void fill_water_to_amount(float amount);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __WATER_DISPENSER_H__ */