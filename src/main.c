#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/rtc_wdt.h"

// libraries for the LCD
#include "EVE_target.h"
#include "EVE_commands.h"
#include "tft.h"

// libaries for reading/writing internal flash memory
#include "memory.h"

// libaries for load cells
#include "load_cell.h"
#include "ultrasonic.h"

#define LED_BUILTIN GPIO_NUM_2

#define LOAD_CELL_NAMESPACE "load_cell_data"
#define ULTRASONIC_NAMESPACE "ultrasonic_data"

void app_main() {
    uint32_t current_millis;
    uint32_t previous_millis = 0;
    uint8_t display_delay = 0;
    uint16_t led = 0;
    uint8_t led_state = 0;
    uint32_t micros_start, micros_end;
    gpio_config_t io_cfg = {0};

    io_cfg.intr_type = GPIO_PIN_INTR_DISABLE;
    io_cfg.mode = GPIO_MODE_OUTPUT;
    io_cfg.pin_bit_mask = BIT(LED_BUILTIN);
    gpio_config(&io_cfg);

    EVE_init_spi();     // initialize LCD as SPI device
    TFT_init();         // run FT813 (LCD graphics chip) initialization sequence
    mem_init();         // initialize read/write for internal memory

    while(1)
    {

       	current_millis = (uint32_t) (esp_timer_get_time() / 1000ULL);

        if((current_millis - previous_millis) > 4) /* execute the code every 5 milli-seconds */
        {
            previous_millis = current_millis;

            led++;
            if(led > 69)
            {
                led = 0;
                if(led_state)
                {
                    led_state = 0;
                    gpio_set_level(LED_BUILTIN, 0);
                }
                else
                {
                    led_state = 42;
                    gpio_set_level(LED_BUILTIN, 1);
                }
            }

            micros_start = (uint32_t) (esp_timer_get_time());
            TFT_touch();
            micros_end = (uint32_t) (esp_timer_get_time());
            num_profile_b = (micros_end - micros_start); /* calculate the micro-seconds passed during the call to TFT_touch */

            display_delay++;
            if(display_delay > 3) /* refresh the display every 20ms */
            {
                display_delay = 0;
                micros_start = (uint32_t) (esp_timer_get_time());
                TFT_display();
                micros_end = (uint32_t) (esp_timer_get_time());
                num_profile_a = (micros_end - micros_start); /* calculate the micro-seconds passed during the call to TFT_display */
            }
        }

        rtc_wdt_feed();
        vPortYield();
    }
}