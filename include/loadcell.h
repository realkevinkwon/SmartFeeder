#ifndef __LOADCELL_H__
#define __LOADCELL_H__

#include <stdio.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hx711.h"

#define LC_NUM_MEASUREMENTS 8
#define LC_NUM_ATTEMPTS 8

#define LC1_DOUT GPIO_NUM_18
#define LC1_PD_SCK GPIO_NUM_19
#define LC1_init() load_cell_init(LC1_DOUT,LC1_PD_SCK)

#define LC1_CALI_ONE 241.0

#define LC2_DOUT GPIO_NUM_5
#define LC2_PD_SCK GPIO_NUM_19
#define LC2_init() load_cell_init(LC2_DOUT,LC2_PD_SCK)

#define LC2_CALI_ONE 234.0

hx711_t load_cell_init(gpio_num_t dout, gpio_num_t pd_sck);
int32_t attempt_load_measure(hx711_t load_cell);
int32_t load_measure(hx711_t load_cell);
void lc1_calibration(hx711_t load_cell);
float lc1_to_grams(int32_t raw);
void lc2_calibration(hx711_t load_cell);
float lc2_to_grams(int32_t raw);
void load_cell_test ();

#endif /* __LOADCELL_H__ */