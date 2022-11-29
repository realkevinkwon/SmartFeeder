# SmartFeeder
ECE 477 Team 14 Project Repository

### Hardware Specifications
| Component       | Name                 |
| --------------- | -------------------- |
| Microcontroller | ESP32-WROOM-32E-N16  |
| LCD             | gen4-FT813-43CTP-CLB |

## Pseudo-code

Program execution begins with ```app_main()```

```main.c```
```python
def app_main():
    EVE_init_spi()
    TFT_init()

    while True:
        run every 5ms
            TFT_touch()
            TFT_display()

        if time.minutes in ['00','15','30','45']:
            ultrasonic_write()
            load_cell_write()
```