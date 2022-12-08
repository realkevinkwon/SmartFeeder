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

// libraries for wi-fi
#include "wifi.h"

// libaries for load cells
#include "loadcell.h"
#include "ultrasonic.h"

void run_display(void* pvParameters) {
    uint32_t current_millis;
    uint32_t previous_millis = 0;
    uint8_t display_delay = 0;
    uint32_t micros_start, micros_end;

    while (1) {
        current_millis = (uint32_t) (esp_timer_get_time() / 1000ULL);

        if((current_millis - previous_millis) > 4) /* execute the code every 5 milli-seconds */
        {
            previous_millis = current_millis;

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

void app_main() {
    TaskHandle_t xHandle = NULL;

    // initialize LCD as SPI device
    printf("\nRunning EVE_init_spi() ... \n");
    EVE_init_spi();
    printf("Done\n");

    // run FT813 (LCD graphics chip) initialization sequence
    printf("Running TFT_init() ... ");
    TFT_init();
    printf("Done\n");

    // initialize read/write for internal memory
    printf("Running mem_initialization() ... ");
    mem_initialization();
    printf("Done\n");

    // initialize read/write for internal memory
    printf("Running wifi_init() ... ");
    wifi_init();
    printf("Done\n");

    setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    tzset();

    xTaskCreate(run_display, "run_display", configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
}