#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#define STEPPER_STEP_PIN GPIO_NUM_4
#define STEPPER_EN_PIN GPIO_NUM_2
#define STEPPER_DIR_PIN GPIO_NUM_16

#define STEP_ANGLE 7.5
#define STEPS_PER_REV (int32_t) (360 / STEP_ANGLE)

#define STEP_RATE 100
#define STEP_PULSE_LENGTH 100

#define step_modulo(steps) (steps % STEPS_PER_REV + STEPS_PER_REV) % STEPS_PER_REV
#define step_degrees(angle, direction) step((int32_t) (angle/STEP_ANGLE), direction)
#define step_to_angle(angle) step_to((int32_t) (angle/STEP_ANGLE))

void stepper_init();
void step(int32_t num_steps, int32_t direction);
void _one_step();
void step_to(int32_t target);

#endif /* __STEPPER_H__ */