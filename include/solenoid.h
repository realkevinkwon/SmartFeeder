#ifndef __SOLENOID_H__
#define __SOLENOID_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#define SOLENOID_PIN GPIO_NUM_15
#define close_solenoid() gpio_set_level(SOLENOID_PIN, 1)
#define open_solenoid() gpio_set_level(SOLENOID_PIN, 0)

#ifdef __cplusplus
extern "C" {
#endif

void solenoid_init();

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __SOLENOID_H__ */