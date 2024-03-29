#include "stepper.h"

// Tag for logging
static const char *STEPPER_TAG = "Stepper";

int32_t steps = 0;

void stepper_init() {
    gpio_reset_pin(STEPPER_STEP_PIN);
    gpio_set_direction(STEPPER_STEP_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(STEPPER_EN_PIN);
    gpio_set_direction(STEPPER_EN_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(STEPPER_DIR_PIN);
    gpio_set_direction(STEPPER_DIR_PIN, GPIO_MODE_OUTPUT);
}

void _one_step() {
    gpio_set_level(STEPPER_STEP_PIN, 1);
    vTaskDelay(STEP_PULSE_LENGTH / portTICK_PERIOD_MS);
    gpio_set_level(STEPPER_STEP_PIN, 0);
    vTaskDelay(STEP_RATE / portTICK_PERIOD_MS);
}

void step(int32_t num_steps, int32_t direction) {
    gpio_set_level(STEPPER_EN_PIN, 1);
    gpio_set_level(STEPPER_DIR_PIN, direction);
    for (int32_t i = 0; i < num_steps; i++) {
        _one_step();
    }
    gpio_set_level(STEPPER_EN_PIN, 0);
    steps += num_steps * (direction ? -1 : 1);
    steps = step_modulo(steps);
}

void step_to(int32_t target, int32_t direction) {
    int32_t step_diff = step_modulo(target - steps);
    // If rotating counter-clockwise, then adjust num_steps
    int32_t num_steps = direction ? STEPS_PER_REV - step_diff : step_diff;
    step(num_steps, direction);
}

void stepper_test() {
    stepper_init();
    while (1) {
        step_angle(45, 1);
        ESP_LOGI(STEPPER_TAG, "Step Count: %d", steps);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        step_angle(52.5, 0);
        ESP_LOGI(STEPPER_TAG, "Step Count: %d", steps);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }   
}