/*
@file    tft.c / tft.cpp
@brief   TFT handling functions for EVE_Test project
@version 1.19
@date    2022-03-19
@author  Rudolph Riedel

@section History

1.14
- added example code for BT81x that demonstrates how to write a flash-image from the microcontrollers memory
  to an external flash on a BT81x module and how to use an UTF-8 font contained in this flash-image

1.15
- moved "display_list_size = EVE_memRead16(REG_CMD_DL);" from TFT_display() to TFT_touch() to speed up the display
 refresh for non-DMA targets

1.16
- disabled the UTF-8 font example code, can be re-enabled if desired by changing the "#define TEST_UTF8 0" to "#define TEST_UTF8 1"

1.17
- replaced the UTF-8 font with a freshly generated one and adjusted the parameters for the .xfont file

1.18
- several minor changes

1.19
- removed most of the history
- changed a couple of "while (EVE_busy()) {};" lines to "EVE_execute_cmd();"
- renamed PINK to MAGENTA

 */

#include <time.h>
#include <sys/time.h>
#include "EVE.h"
#include "tft_data.h"



/* ############################# MACROS - BEGIN ############################# */
#define TEST_UTF8 0
#define DISPLAY_ORIENTATION 0
#define MEM_DL_STATIC (EVE_RAM_G_SIZE - 4096) /* 0xff000 - start-address of the static part of the display-list, upper 4k of gfx-mem */


/* === pre-defined colors === */
#define RED     0xff0000UL
#define ORANGE  0xffa500UL
#define GREEN   0x00ff00UL
#define BLUE    0x0000ffUL
#define BABY_BLUE  0x5dade2L
#define BABY_YELLOW 0xffee84UL
#define YELLOW  0xffff00UL
#define MAGENTA 0xff00ffUL
#define PURPLE  0x800080UL
#define WHITE   0xffffffUL
#define BLACK   0x000000UL
/* ========================== */


/* === delay values for touch === */
#define DELAY_BUTTON 5
#define DELAY_KEY 3
/* ============================== */


/* === macro functions for input === */
#define KEY_VALUE(tag) (tag - KEY_TAG_OFFSET)
#define INT_TO_ASCII(x) (x + 48)
/* ================================= */

/* === screenstate values === */
#define SCREENSTATE_HOME 0
#define SCREENSTATE_DATA 1
#define SCREENSTATE_SCHEDULE 2
#define SCREENSTATE_SETTINGS 3
/* ========================== */


/* === tag values for touch === */
/* home screen */
#define TAG_HOME_DATABUTTON 1
#define TAG_HOME_SCHEDULEBUTTON 2
#define TAG_HOME_SETTINGSBUTTON 3

/* data screen */
#define TAG_DATA_BACKBUTTON 4

/* schedule screen */
#define TAG_SCHEDULE_BACKBUTTON 5
#define KEY_TAG_OFFSET 6
#define TAG_SCHEDULE_KEY0 (KEY_TAG_OFFSET + 0)
#define TAG_SCHEDULE_KEY1 (KEY_TAG_OFFSET + 1)
#define TAG_SCHEDULE_KEY2 (KEY_TAG_OFFSET + 2)
#define TAG_SCHEDULE_KEY3 (KEY_TAG_OFFSET + 3)
#define TAG_SCHEDULE_KEY4 (KEY_TAG_OFFSET + 4)
#define TAG_SCHEDULE_KEY5 (KEY_TAG_OFFSET + 5)
#define TAG_SCHEDULE_KEY6 (KEY_TAG_OFFSET + 6)
#define TAG_SCHEDULE_KEY7 (KEY_TAG_OFFSET + 7)
#define TAG_SCHEDULE_KEY8 (KEY_TAG_OFFSET + 8)
#define TAG_SCHEDULE_KEY9 (KEY_TAG_OFFSET + 9)

/* settings screen */
#define TAG_SETTINGS_BACKBUTTON 16
/* ======================== */


/* === macros for drawing graphs === */
// location of bottom-left of graph on the LCD
#define GRAPH_X_BASE 200
#define GRAPH_Y_BASE 220

// with scale set to 1, each unit corresponds to 1 pixel
#define GRAPH_X_SCALE 1
#define GRAPH_Y_SCALE 1

// length of the x and y axes
#define GRAPH_X_LEN 200
#define GRAPH_Y_LEN 150

// interval between gridlines
#define GRAPH_X_INTERVAL (GRAPH_X_SCALE * 50)
#define GRAPH_Y_INTERVAL (GRAPH_Y_SCALE * 50)
/* ================================= */


/* === dimensions and locations of on-screen elements === */
/* home screen buttons */
#define HOME_BUTTON_WIDTH 110
#define HOME_BUTTON_HEIGHT 110
#define HOME_BUTTON_X 20
#define HOME_BUTTON_Y 60

/* digital clock */
#define DIGIT_WIDTH 16
#define CLOCK_X1 340
#define CLOCK_X2 (CLOCK_X1 + 38)
#define CLOCK_X3 (CLOCK_X2 + 40)
#define CLOCK_Y 2

/* number keys */
#define KEY_HEIGHT 30
#define KEY_WIDTH 30
#define KEYPAD_X 340
#define KEYPAD_Y 50
#define KEY_X_OFFSET (KEY_WIDTH + 10)
#define KEY_Y_OFFSET (KEY_HEIGHT + 10)
/* ====================================================== */


/* === fonts === */
#define FONT_PRIMARY 28
#define FONT_TIME 30
/* ============= */


/* === memory-map addresses for bitmaps === */
#define MEM_PIC_WIFI 0x000f0000
/* ======================================== */
/* ############################## MACROS - END ############################## */



/* ######################## GLOBAL VARIABLES - BEGIN ######################## */
/* === general variables === */
uint32_t num_dl_static; /* amount of bytes in the static part of our display-list */
uint8_t tft_active = 0;
uint16_t num_profile_a, num_profile_b;
uint16_t display_list_size = 0;
/* ========================= */


/* === variables for input === */
char input[255] = "";
size_t input_size = 0;
/* =========================== */


/* === variables for touch === */
uint16_t toggle_state[255];                 // holds toggle states for various ui elements
uint8_t screen_state = SCREENSTATE_HOME;    // holds which screen should be displayed
uint16_t lock_delay = 0;                    // delay for buttons
uint8_t toggle_lock = 0;                    // allows only one touch target to be activated at a time
/* =========================== */


/* === variables for drawing graphs === */
uint16_t num_points = 8;
/* 
    data MUST be in order:
    x_data = {x_0, x_1, x_2,...,x_i, x_i+1,...}
    y_data = {y_0, y_1, y_2,...,y_i, y_i+1,...}
*/
int16_t x_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int16_t y_data[8] = {0, 10, 40, 35, 70, 11, 16, 28};
/* ==================================== */
/* ######################### GLOBAL VARIABLES - END ######################### */



/* === functions for initialization === */
void touch_calibrate(void) {
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00000CC1);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x0000FE4c);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFF6BA43);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x000115DF);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0xFFFFF849);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFF8DE4B);

// activate this if you are using a module for the first time or if you need to re-calibrate it
// write down the numbers on the screen and make a new block
#if 0
    /* calibrate touch and displays values to screen */
    EVE_cmd_dl(CMD_DLSTART);
    EVE_cmd_dl(DL_CLEAR_RGB | BLACK);
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    EVE_cmd_text((EVE_HSIZE/2), 50, 26, EVE_OPT_CENTER, "Please tap on the dot.");
    EVE_cmd_calibrate();
    EVE_cmd_dl(DL_DISPLAY);
    EVE_cmd_dl(CMD_SWAP);
    EVE_execute_cmd();

    uint32_t touch_a, touch_b, touch_c, touch_d, touch_e, touch_f;

    touch_a = EVE_memRead32(REG_TOUCH_TRANSFORM_A);
    touch_b = EVE_memRead32(REG_TOUCH_TRANSFORM_B);
    touch_c = EVE_memRead32(REG_TOUCH_TRANSFORM_C);
    touch_d = EVE_memRead32(REG_TOUCH_TRANSFORM_D);
    touch_e = EVE_memRead32(REG_TOUCH_TRANSFORM_E);
    touch_f = EVE_memRead32(REG_TOUCH_TRANSFORM_F);

    EVE_cmd_dl(CMD_DLSTART);
    EVE_cmd_dl(DL_CLEAR_RGB | BLACK);
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    EVE_cmd_dl(TAG(0));

    EVE_cmd_text(5, 15, 26, 0, "TOUCH_TRANSFORM_A:");
    EVE_cmd_text(5, 30, 26, 0, "TOUCH_TRANSFORM_B:");
    EVE_cmd_text(5, 45, 26, 0, "TOUCH_TRANSFORM_C:");
    EVE_cmd_text(5, 60, 26, 0, "TOUCH_TRANSFORM_D:");
    EVE_cmd_text(5, 75, 26, 0, "TOUCH_TRANSFORM_E:");
    EVE_cmd_text(5, 90, 26, 0, "TOUCH_TRANSFORM_F:");

    EVE_cmd_setbase(16L);
    EVE_cmd_number(310, 15, 26, EVE_OPT_RIGHTX|8, touch_a);
    EVE_cmd_number(310, 30, 26, EVE_OPT_RIGHTX|8, touch_b);
    EVE_cmd_number(310, 45, 26, EVE_OPT_RIGHTX|8, touch_c);
    EVE_cmd_number(310, 60, 26, EVE_OPT_RIGHTX|8, touch_d);
    EVE_cmd_number(310, 75, 26, EVE_OPT_RIGHTX|8, touch_e);
    EVE_cmd_number(310, 90, 26, EVE_OPT_RIGHTX|8, touch_f);

    EVE_cmd_dl(DL_DISPLAY); /* instruct the co-processor to show the list */
    EVE_cmd_dl(CMD_SWAP); /* make this list active */
    EVE_execute_cmd();

    while(1);
#endif
}

void EVE_cmd_loadimages(void) {
    EVE_cmd_loadimage(MEM_PIC_WIFI, EVE_OPT_NODL, pic_wifi_32, sizeof(pic_wifi_32));
}

void TFT_init(void)
{
    if(E_OK == EVE_init())
    {
        tft_active = 1;
        for (int i = 0; i < 255; i++) {
            toggle_state[i] = 0;
        }

        EVE_memWrite8(REG_PWM_DUTY, 0x30);  /* setup backlight, range is from 0 = off to 0x80 = max */

        touch_calibrate();

        EVE_cmd_loadimages();
        
        EVE_cmd_setrotate(DISPLAY_ORIENTATION);
    }
}
/* ==================================== */

/* check for touch events and setup vars for TFT_display() */
void TFT_touch(void) {
    uint8_t tag;
    // static uint8_t toggle_lock = 0;

    if(tft_active != 0)
    {
        if(EVE_IS_BUSY == EVE_busy()) /* is EVE still processing the last display list? */
        {
            return;
        }

        display_list_size = EVE_memRead16(REG_CMD_DL); /* debug-information, get the size of the last generated display-list */

        tag = EVE_memRead8(REG_TOUCH_TAG); /* read the value for the first touch point */

        switch(tag)
        {
            case 0:
                if (lock_delay == 0) {
                    toggle_lock = 0;
                }
                break;

            case TAG_HOME_DATABUTTON: /* use button on top as on/off toggle-switch */
            case TAG_HOME_SCHEDULEBUTTON:
            case TAG_HOME_SETTINGSBUTTON:
            case TAG_DATA_BACKBUTTON:
            case TAG_SCHEDULE_BACKBUTTON:
            case TAG_SETTINGS_BACKBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_BUTTON;
                }
                break;
            case TAG_SCHEDULE_KEY0:
            case TAG_SCHEDULE_KEY1:
            case TAG_SCHEDULE_KEY2:
            case TAG_SCHEDULE_KEY3:
            case TAG_SCHEDULE_KEY4:
            case TAG_SCHEDULE_KEY5:
            case TAG_SCHEDULE_KEY6:
            case TAG_SCHEDULE_KEY7:
            case TAG_SCHEDULE_KEY8:
            case TAG_SCHEDULE_KEY9:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    input[input_size] = INT_TO_ASCII(KEY_VALUE(tag));
                    input_size++;
                    input[input_size] = '\0';
                    lock_delay = DELAY_KEY;
                }
                break;
        }
    }
}

void EVE_cmd_bitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_setbitmap_burst(addr, fmt, width, height);
    EVE_cmd_dl_burst(VERTEX2F(x * 16, y * 16));
    EVE_cmd_dl_burst(DL_END);
}

void EVE_cmd_statusbar_burst(void) {
    // status bar background
    EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
    EVE_color_rgb_burst(BABY_BLUE);
    EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl_burst(VERTEX2F(0 * 16, 0 * 16));
    EVE_cmd_dl_burst(VERTEX2F(480 * 16, 40 * 16));

    // Wi-Fi symbol
    EVE_color_rgb_burst(WHITE);
    EVE_cmd_bitmap_burst(MEM_PIC_WIFI, EVE_ARGB4, 32, 32, 10, 5);

    // clock
    EVE_cmd_number_burst(CLOCK_X1, CLOCK_Y, FONT_TIME, 0, 0);
    EVE_cmd_number_burst(CLOCK_X1 + DIGIT_WIDTH, CLOCK_Y, FONT_TIME, 0, 9);
    EVE_cmd_text_burst(CLOCK_X2 - 5, CLOCK_Y + 1, 24, 0, ":");
    EVE_cmd_number_burst(CLOCK_X2, CLOCK_Y, FONT_TIME, 0, 2);
    EVE_cmd_number_burst(CLOCK_X2 + DIGIT_WIDTH, CLOCK_Y, FONT_TIME, 0, 8);
    EVE_cmd_text_burst(CLOCK_X3, CLOCK_Y + 0, FONT_TIME, 0, "PM");
}

int16_t getMinValue(int16_t* arr, uint16_t num_points) {
    int16_t min_value = arr[0];
    for (int i = 0; i < num_points; i++) {
        min_value = arr[i] < min_value ? arr[i] : min_value;
    }
    return min_value;
}

int16_t getMaxValue(int16_t* arr, uint16_t num_points) {
    int16_t max_value = arr[0];
    for (int i = 1; i < num_points; i++) {
        max_value = arr[i] > max_value ? arr[i] : max_value;
    }
    return max_value;
}

void scaleData(int16_t* x_data, int16_t* y_data, uint16_t num_points) {

    return;
}

void EVE_cmd_display_graph_burst(int16_t* x_data, int16_t* y_data, uint16_t num_points) {
    EVE_color_rgb_burst(COLOR_RGB(100,100,100));
    EVE_cmd_dl_burst(LINE_WIDTH(8));

    // draw the x-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_X_BASE * 16, GRAPH_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + GRAPH_X_LEN) * 16, GRAPH_Y_BASE * 16));

    // draw the y-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_X_BASE * 16, GRAPH_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_X_BASE * 16, (GRAPH_Y_BASE - GRAPH_Y_LEN) * 16));

    // draw horizontal gridlines
    EVE_color_rgb_burst(COLOR_RGB(200,200,200));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_X_BASE * 16, (GRAPH_Y_BASE - GRAPH_Y_INTERVAL) * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + GRAPH_X_LEN) * 16, (GRAPH_Y_BASE - GRAPH_Y_INTERVAL) * 16));
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_X_BASE * 16, (GRAPH_Y_BASE - 2 * GRAPH_Y_INTERVAL) * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + GRAPH_X_LEN) * 16, (GRAPH_Y_BASE - 2 * GRAPH_Y_INTERVAL) * 16));
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_X_BASE * 16, (GRAPH_Y_BASE -  3 * GRAPH_Y_INTERVAL) * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + GRAPH_X_LEN) * 16, (GRAPH_Y_BASE - 3 * GRAPH_Y_INTERVAL) * 16));

    // draw vertical gridlines
    EVE_color_rgb_burst(COLOR_RGB(200,200,200));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + GRAPH_X_INTERVAL) * 16, GRAPH_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + GRAPH_X_INTERVAL) * 16, (GRAPH_Y_BASE - GRAPH_Y_LEN) * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + 2 * GRAPH_X_INTERVAL) * 16, GRAPH_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + 2 * GRAPH_X_INTERVAL) * 16, (GRAPH_Y_BASE - GRAPH_Y_LEN) * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + 3 * GRAPH_X_INTERVAL) * 16, GRAPH_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + 3 * GRAPH_X_INTERVAL) * 16, (GRAPH_Y_BASE - GRAPH_Y_LEN) * 16));

    // plot data
    EVE_color_rgb_burst(BLACK);
    EVE_cmd_dl_burst(LINE_WIDTH(12));
    for (int i = 0; i < (num_points - 1); i++) {
        EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + x_data[i]) * 16, (GRAPH_Y_BASE - y_data[i]) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_X_BASE + x_data[i+1]) * 16, (GRAPH_Y_BASE - y_data[i+1]) * 16));
    }
}

void EVE_cmd_custombutton_burst(uint8_t tag_value) {

    switch (tag_value) {
        case TAG_HOME_DATABUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(HOME_BUTTON_X, HOME_BUTTON_Y, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, 28, toggle_state[tag_value], " ");
            EVE_cmd_text_burst(HOME_BUTTON_X + 10, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT - 30, FONT_PRIMARY, 0, "Data");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_HOME_SCHEDULEBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(2 * HOME_BUTTON_X + HOME_BUTTON_WIDTH, HOME_BUTTON_Y, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_text_burst(2 * HOME_BUTTON_X + HOME_BUTTON_WIDTH + 10, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT - 30, FONT_PRIMARY, 0, "Schedule");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_HOME_SETTINGSBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(3 * HOME_BUTTON_X + 2 * HOME_BUTTON_WIDTH, HOME_BUTTON_Y, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_text_burst(3 * HOME_BUTTON_X + 2 * HOME_BUTTON_WIDTH + 10, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT - 30, FONT_PRIMARY, 0, "Settings");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY1:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "1");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY2:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "2");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY3:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "3");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY4:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y + KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "4");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY5:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y + KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "5");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY6:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y + KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "6");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY7:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y + 2 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "7");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY8:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y + 2 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "8");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY9:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y + 2 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "9");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY0:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y + 3 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "0");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_BACKBUTTON:
        case TAG_SCHEDULE_BACKBUTTON:
        case TAG_SETTINGS_BACKBUTTON:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(20, 220, 30, 30, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
            EVE_cmd_dl_burst(DL_BEGIN | EVE_LINE_STRIP);
            EVE_cmd_dl_burst(VERTEX2F(32 * 16, 236 * 16));
            EVE_cmd_dl_burst(VERTEX2F(36 * 16, 232 * 16));
            EVE_cmd_dl_burst(VERTEX2F(36 * 16, 240 * 16));
            EVE_cmd_dl_burst(VERTEX2F(32 * 16, 236 * 16));
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

void EVE_cmd_keypad_burst(void) {
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY1);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY2);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY3);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY4);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY5);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY6);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY7);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY8);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY9);
    EVE_cmd_custombutton_burst(TAG_SCHEDULE_KEY0);
}

/* === screens === */
void TFT_home(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay > 1) {
            lock_delay--;
        }
        else if (lock_delay == 1) {
            if (toggle_state[TAG_HOME_DATABUTTON] != 0) {
                screen_state = SCREENSTATE_DATA;
                toggle_state[TAG_HOME_DATABUTTON] = 0;
            }
            else if (toggle_state[TAG_HOME_SCHEDULEBUTTON] != 0) {
                screen_state = SCREENSTATE_SCHEDULE;
                toggle_state[TAG_HOME_SCHEDULEBUTTON] = 0;
            }
            else if (toggle_state[TAG_HOME_SETTINGSBUTTON] != 0) {
                screen_state = SCREENSTATE_SETTINGS;
                toggle_state[TAG_HOME_SETTINGSBUTTON] = 0;
            }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_custombutton_burst(TAG_HOME_DATABUTTON);

        EVE_cmd_custombutton_burst(TAG_HOME_SCHEDULEBUTTON);

        EVE_cmd_custombutton_burst(TAG_HOME_SETTINGSBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

void TFT_data(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay > 1) {
            lock_delay--;
        }
        else if (lock_delay == 1) {
            if (toggle_state[TAG_DATA_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_HOME;
                toggle_state[TAG_DATA_BACKBUTTON] = 0;
            }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_display_graph_burst(x_data, y_data, num_points);

        EVE_cmd_custombutton_burst(TAG_DATA_BACKBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

void TFT_schedule(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay > 1) {
            lock_delay--;
        }
        else if (lock_delay == 1) {
            if (toggle_state[TAG_SCHEDULE_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_HOME;
                toggle_state[TAG_SCHEDULE_BACKBUTTON] = 0;
            }
            else if (toggle_state[TAG_SCHEDULE_KEY0] != 0) { toggle_state[TAG_SCHEDULE_KEY0] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY1] != 0) { toggle_state[TAG_SCHEDULE_KEY1] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY2] != 0) { toggle_state[TAG_SCHEDULE_KEY2] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY3] != 0) { toggle_state[TAG_SCHEDULE_KEY3] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY4] != 0) { toggle_state[TAG_SCHEDULE_KEY4] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY5] != 0) { toggle_state[TAG_SCHEDULE_KEY5] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY6] != 0) { toggle_state[TAG_SCHEDULE_KEY6] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY7] != 0) { toggle_state[TAG_SCHEDULE_KEY7] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY8] != 0) { toggle_state[TAG_SCHEDULE_KEY8] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY9] != 0) { toggle_state[TAG_SCHEDULE_KEY9] = 0; }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_custombutton_burst(TAG_SCHEDULE_BACKBUTTON);

        EVE_cmd_keypad_burst();

        EVE_color_rgb_burst(BLACK);
        EVE_cmd_text_burst(50, 50, FONT_PRIMARY, 0, input);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

void TFT_settings(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay > 1) {
            lock_delay--;
        }
        else if (lock_delay == 1) {
            if (toggle_state[TAG_SETTINGS_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_HOME;
                toggle_state[TAG_SETTINGS_BACKBUTTON] = 0;
            }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_custombutton_burst(TAG_SETTINGS_BACKBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}
/* =============== */


/* dynamic portion of display-handling, meant to be called every 20ms or more */
void TFT_display(void) {
    switch (screen_state) {
        case SCREENSTATE_HOME:
            TFT_home();
            break;
        case SCREENSTATE_DATA:
            TFT_data();
            break;
        case SCREENSTATE_SETTINGS:
            TFT_settings();
            break;
        case SCREENSTATE_SCHEDULE:
            TFT_schedule();
            break;
    }
}

/* TEMPLATE */
// void TFT_display(void) {
//     if (tft_active != 0) {
//         EVE_start_cmd_burst();
//         EVE_cmd_dl_burst(CMD_DLSTART);
//         EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
//         EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
//         EVE_cmd_dl_burst(TAG(0));
//
//         EVE_cmd_dl_burst(DL_DISPLAY);
//         EVE_cmd_dl_burst(CMD_SWAP);
//         EVE_end_cmd_burst();
//         while (EVE_busy());
//     }
// }