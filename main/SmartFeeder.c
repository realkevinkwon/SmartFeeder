#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/gpio_reg.h"

#define GPIO_OUTPUT_PIN_SEL ((1<<GPIO_NUM_21) | (1<<GPIO_NUM_17) | (1<<GPIO_NUM_16) | (1<<GPIO_NUM_19))
#define GPIO_OUTPUT_IO_0 0
#define GPIO_OUTPUT_IO_1 1
#define PIN_0 21
#define PIN_1 17
#define PIN_2 16
#define PIN_3 19
#define STEPS_PER_REVOLUTION 1600
#define DELAY 10

#define STEP0 (0x1<<PIN_0)
#define STEP1 (0x1<<PIN_0)|(1<<PIN_1)
#define STEP2 (0x1<<PIN_1)
#define STEP3 (0x1<<PIN_1)|(1<<PIN_2)
#define STEP4 (0x1<<PIN_2)
#define STEP5 (0x1<<PIN_2)|(1<<PIN_3)
#define STEP6 (0x1<<PIN_3)
#define STEP7 (0x1<<PIN_3)|(1<<PIN_0)

void gpio_setup(gpio_config_t* io_conf) {
    // disable interrupt
    io_conf->intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf->mode = GPIO_MODE_OUTPUT;
    // bit mask of pins to enable
    io_conf->pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf->pull_down_en = GPIO_PULLDOWN_DISABLE;
    // disable pull-up mode
    io_conf->pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(io_conf);
}

void app_main(void)
{
    // zero-initialize the config structure
    gpio_config_t io_conf = {};
    gpio_setup(&io_conf);

    uint32_t steps[8] = {STEP0, STEP1, STEP2, STEP3, STEP4, STEP5, STEP6, STEP7};

    for (int i = 0; i < 100000; i++) {
        printf("%d\n", i);
        REG_WRITE(GPIO_OUT_REG, steps[i % 8]);
        vTaskDelay(DELAY / portTICK_PERIOD_MS);
    }
}