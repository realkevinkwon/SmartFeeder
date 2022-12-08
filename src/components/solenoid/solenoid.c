#include "solenoid.h"

// Tag for logging
// static const char *TAG = "Solenoid";

void solenoid_init() {
    // Configure the gpio pin as output
    gpio_reset_pin(SOLENOID_PIN);
    gpio_set_direction(SOLENOID_PIN, GPIO_MODE_OUTPUT);
    close_solenoid();
}