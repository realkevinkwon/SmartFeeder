# SmartFeeder
ECE 477 Team 14 Project Repository

### Hardware Specifications
| Component       | Name                 |
| --------------- | -------------------- |
| Microcontroller | ESP32-WROOM-32E-N16  |
| LCD             | gen4-FT813-43CTP-CLB |

## Pseudo-code

Program execution begins with ```app_main()```

```python
# main.c
def app_main():
    EVE_init_spi()
    TFT_init()

    while True:
        # runs every 5ms
        if time_passed == 5ms:
            TFT_touch()
            TFT_display()

        # write data from ultrasonic sensors and load cells to internal memory
        if time.minutes in ['00','15','30','45']:
            ultrasonic_write()
            load_cell_write()

# tft.c
def TFT_display():
    # 'screen_state' determines which screen is displayed
    switch (screen_state):
        case SCREENSTATE_HOME:
            TFT_home()
            break
        case SCREENSTATE_DATA:
            TFT_data()
            break
        case SCREENSTATE_SETTINGS:
            TFT_settings()
            break
        case SCREENSTATE_SCHEDULE:
            TFT_schedule()
            break
```