#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "EVE.h"
#include "tft_data.h"
#include "memory.h"
#include "clock.h"

#include "food_dispenser.h"
#include "water_dispenser.h"


/* ############################# MACROS - BEGIN ############################# */
#define TEST_UTF8 0
#define DISPLAY_ORIENTATION 0
#define MEM_DL_STATIC (EVE_RAM_G_SIZE - 4096) /* 0xff000 - start-address of the static part of the display-list, upper 4k of gfx-mem */


/* === macros for reading from and writing to internal memory === */
#define SETTINGS_NAMESPACE "settings"       // general system settings
#define SCHEDULE_NAMESPACE "schedule"       // saved feeding schedule
#define WATER_NAMESPACE "water"             // water consumption data
#define FOOD_NAMESPACE "food"               // food consumption data
/* ============================================================== */


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
#define ASCII_TO_INT(x) (x - 48)
/* ================================= */

/* === screenstate values === */
#define SCREENSTATE_HOME 0
#define SCREENSTATE_DATA 1
#define SCREENSTATE_SCHEDULE 2
#define SCREENSTATE_SETTINGS 3
#define SCREENSTATE_DEMO 4
#define SCREENSTATE_TIME 5
/* ========================== */


/* === tag values for touch === */
#define TAG_SIZE 40
/* home screen */
#define TAG_OFFSET_HOME 1
#define TAG_HOME_DATABUTTON (TAG_OFFSET_HOME + 0)
#define TAG_HOME_SCHEDULEBUTTON (TAG_OFFSET_HOME + 1)
#define TAG_HOME_SETTINGSBUTTON (TAG_OFFSET_HOME + 2)

/* data screen */
#define TAG_OFFSET_DATA (TAG_SIZE + 1)
#define TAG_DATA_BACKBUTTON (TAG_OFFSET_DATA + 0)
#define TAG_DATA_START_MONTH_UP (TAG_OFFSET_DATA + 1)
#define TAG_DATA_START_MONTH_DOWN (TAG_OFFSET_DATA + 2)
#define TAG_DATA_START_DAY_UP (TAG_OFFSET_DATA + 3)
#define TAG_DATA_START_DAY_DOWN (TAG_OFFSET_DATA + 4)
#define TAG_DATA_START_YEAR_UP (TAG_OFFSET_DATA + 5)
#define TAG_DATA_START_YEAR_DOWN (TAG_OFFSET_DATA + 6)
#define TAG_DATA_END_MONTH_UP (TAG_OFFSET_DATA + 7)
#define TAG_DATA_END_MONTH_DOWN (TAG_OFFSET_DATA + 8)
#define TAG_DATA_END_DAY_UP (TAG_OFFSET_DATA + 9)
#define TAG_DATA_END_DAY_DOWN (TAG_OFFSET_DATA + 10)
#define TAG_DATA_END_YEAR_UP (TAG_OFFSET_DATA + 11)
#define TAG_DATA_END_YEAR_DOWN (TAG_OFFSET_DATA + 12)

#define TAG_DATA_VIEWBUTTON (TAG_OFFSET_DATA + 13)

/* schedule screen */
#define TAG_OFFSET_SCHEDULE (2 * TAG_SIZE + 1)
#define TAG_SCHEDULE_BACKBUTTON (TAG_OFFSET_SCHEDULE + 0)

#define TAG_SCHEDULE_KEY_ENTER (TAG_OFFSET_SCHEDULE + 1)
#define TAG_SCHEDULE_KEY_CLEAR (TAG_OFFSET_SCHEDULE + 2)

#define TAG_SCHEDULE_READBUTTON (TAG_OFFSET_SCHEDULE + 3)

#define KEY_TAG_OFFSET (TAG_OFFSET_SCHEDULE + 4)
#define TAG_SCHEDULE_KEY_0 (KEY_TAG_OFFSET + 0)
#define TAG_SCHEDULE_KEY_1 (KEY_TAG_OFFSET + 1)
#define TAG_SCHEDULE_KEY_2 (KEY_TAG_OFFSET + 2)
#define TAG_SCHEDULE_KEY_3 (KEY_TAG_OFFSET + 3)
#define TAG_SCHEDULE_KEY_4 (KEY_TAG_OFFSET + 4)
#define TAG_SCHEDULE_KEY_5 (KEY_TAG_OFFSET + 5)
#define TAG_SCHEDULE_KEY_6 (KEY_TAG_OFFSET + 6)
#define TAG_SCHEDULE_KEY_7 (KEY_TAG_OFFSET + 7)
#define TAG_SCHEDULE_KEY_8 (KEY_TAG_OFFSET + 8)
#define TAG_SCHEDULE_KEY_9 (KEY_TAG_OFFSET + 9)

/* settings screen */
#define TAG_OFFSET_SETTINGS (3 * TAG_SIZE + 1)
#define TAG_SETTINGS_BACKBUTTON (TAG_OFFSET_SETTINGS + 0)
#define TAG_SETTINGS_ERASEBUTTON (TAG_OFFSET_SETTINGS + 1)
#define TAG_SETTINGS_TIMEBUTTON (TAG_OFFSET_SETTINGS + 2)
#define TAG_SETTINGS_DEMOBUTTON (TAG_OFFSET_SETTINGS + 3)

/* demo screen */
#define TAG_OFFSET_DEMO (4 * TAG_SIZE + 1)
#define TAG_DEMO_BACKBUTTON (TAG_OFFSET_DEMO + 0)
#define TAG_DEMO_WATERBUTTON (TAG_OFFSET_DEMO + 1)
#define TAG_DEMO_FOODBUTTON (TAG_OFFSET_DEMO + 2)

/* time screen */
#define TAG_OFFSET_TIME (5 * TAG_SIZE + 1)
#define TAG_TIME_BACKBUTTON (TAG_OFFSET_TIME + 0)
#define TAG_TIME_RESETBUTTON (TAG_OFFSET_TIME + 1)
#define TAG_TIME_SETBUTTON (TAG_OFFSET_TIME + 2)
#define TAG_TIME_MONTH_UP (TAG_OFFSET_TIME + 3)
#define TAG_TIME_MONTH_DOWN (TAG_OFFSET_TIME + 4)
#define TAG_TIME_DAY_UP (TAG_OFFSET_TIME + 5)
#define TAG_TIME_DAY_DOWN (TAG_OFFSET_TIME + 6)
#define TAG_TIME_YEAR_UP (TAG_OFFSET_TIME + 7)
#define TAG_TIME_YEAR_DOWN (TAG_OFFSET_TIME + 8)
#define TAG_TIME_HOUR_UP (TAG_OFFSET_TIME + 9)
#define TAG_TIME_HOUR_DOWN (TAG_OFFSET_TIME + 10)
#define TAG_TIME_MINUTE_UP (TAG_OFFSET_TIME + 11)
#define TAG_TIME_MINUTE_DOWN (TAG_OFFSET_TIME + 12)
#define TAG_TIME_AMPM_UP (TAG_OFFSET_TIME + 13)
#define TAG_TIME_AMPM_DOWN (TAG_OFFSET_TIME + 14)
#define TAG_TIME_12HR_TOGGLE (TAG_OFFSET_TIME + 15)
/* ======================== */


/* === macros for drawing graphs === */
// with scale set to 100, each unit corresponds to 1 pixel
#define GRAPH_X_SCALE 100
#define GRAPH_Y_SCALE 100

// length of the x and y axes
#define GRAPH_X_LEN 200
#define GRAPH_Y_LEN 100

// location of bottom-left of graph on the LCD
#define GRAPH_FOOD_X_BASE 270
#define GRAPH_FOOD_Y_BASE 135
#define GRAPH_WATER_X_BASE GRAPH_FOOD_X_BASE
#define GRAPH_WATER_Y_BASE (GRAPH_FOOD_Y_BASE + GRAPH_Y_LEN + 10)
/* ================================= */


/* === dimensions and locations of on-screen elements === */
/* home screen buttons */
#define HOME_BUTTON_WIDTH 200
#define HOME_BUTTON_HEIGHT 40
#define HOME_BUTTON_X 20
#define HOME_BUTTON_Y 45

/* settings screen buttons */
#define SETTINGS_BUTTON_WIDTH 200
#define SETTINGS_BUTTON_HEIGHT 40
#define SETTINGS_BUTTON_X 20
#define SETTINGS_BUTTON_Y 45

/* data screen buttons */
#define DATA_OFFSET 133
#define DATA_DATE_BUTTON_HEIGHT 20
#define DATA_DATE_BUTTON_WIDTH 24
#define DATA_X1 START_DATE_X
#define DATA_X2 (DATA_X1 + 35)
#define DATA_X3 (DATA_X2 + 35)
#define DATA_X4 END_DATE_X
#define DATA_X5 (DATA_X4 + 35)
#define DATA_X6 (DATA_X5 + 35)

#define DATA_Y1 (START_DATE_Y - 35)
#define DATA_Y2 (START_DATE_Y + 30)

#define DATA_VIEWBUTTON_WIDTH 120
#define DATA_VIEWBUTTON_HEIGHT 30
#define DATA_VIEWBUTTON_X 80
#define DATA_VIEWBUTTON_Y 220

/* time screen */
#define TIME_TEXT_X 100
#define TIME_TEXT_Y 120
#define TIME_TEXT_WIDTH 280
#define TIME_TEXT_HEIGHT 25
#define TIME_BUTTON_WIDTH DATA_DATE_BUTTON_WIDTH
#define TIME_BUTTON_HEIGHT DATA_DATE_BUTTON_HEIGHT

#define TIME_SETBUTTON_X 160
#define TIME_SETBUTTON_Y 220
#define TIME_SETBUTTON_WIDTH 150
#define TIME_SETBUTTON_HEIGHT 30

#define TIME_X1 TIME_TEXT_X + 8
#define TIME_X2 (TIME_X1 + 52)
#define TIME_X3 (TIME_X2 + 35)
#define TIME_X4 (TIME_X3 + 70)
#define TIME_X5 (TIME_X4 + 38)
#define TIME_X6 (TIME_X5 + 35)

#define TIME_Y1 (TIME_TEXT_Y - 33)
#define TIME_Y2 (TIME_TEXT_Y + 37)

/* demo screen */
#define DEMO_BUTTON_X 20
#define DEMO_BUTTON_Y 50
#define DEMO_BUTTON_WIDTH 200
#define DEMO_BUTTON_HEIGHT 60

/* back button */
#define BACK_BUTTON_WIDTH 30
#define BACK_BUTTON_HEIGHT BACK_BUTTON_WIDTH
#define BACK_BUTTON_X 20
#define BACK_BUTTON_Y 220

/* = status bar = */
#define STATUS_BAR_HEIGHT 25

/* wifi status */
#define WIFI_WIDTH 32
#define WIFI_HEIGHT WIFI_WIDTH
#define WIFI_X 10
#define WIFI_Y 1

/* digital clock */
#define DIGIT_WIDTH 12
#define CLOCK_X1 385
#define CLOCK_X2 (CLOCK_X1 + 28)
#define CLOCK_X3 (CLOCK_X2 + 30)
#define CLOCK_Y 1

/* date */
#define DATE_MONTH_X 10
#define DATE_DAY_X (DATE_MONTH_X + 55)
#define DATE_YEAR_X (DATE_DAY_X + 35)
#define DATE_Y CLOCK_Y
/* ============== */

/* number keys */
#define KEY_HEIGHT 45
#define KEY_WIDTH 45
#define KEYPAD_X 310
#define KEYPAD_Y 50
#define KEY_X_OFFSET (KEY_WIDTH + 10)
#define KEY_Y_OFFSET (KEY_HEIGHT + 10)

/* dates */
#define START_DATE_X 10
#define START_DATE_Y 90
#define END_DATE_X (START_DATE_X + DATE_WIDTH + 20)
#define END_DATE_Y START_DATE_Y
#define DATE_HEIGHT 16
#define MONTH_WIDTH 24
#define MONTH_HEIGHT DATE_HEIGHT
#define DAY_WIDTH 16
#define DAY_HEIGHT DATE_HEIGHT
#define YEAR_WIDTH 32
#define YEAR_HEIGHT DATE_HEIGHT
#define DATE_WIDTH (MONTH_WIDTH + DAY_WIDTH + YEAR_WIDTH + 30)
/* ====================================================== */


/* === fonts === */
#define FONT_PRIMARY 28
#define FONT_KEY_WORD 20
#define FONT_TIME 28
#define FONT_DATE 26
/* ============= */


#define BUF_SIZE 256


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


/* === variables for input and output === */
char input[255] = "";
size_t input_length = 0;
char output[255] = "";
size_t output_length = 0;
/* =========================== */


/* === variables for touch === */
uint16_t toggle_state[255];                 // holds toggle states for various ui elements
uint8_t screen_state = SCREENSTATE_HOME;    // holds which screen should be displayed
uint16_t lock_delay = 0;                    // delay for buttons
uint8_t toggle_lock = 0;                    // allows only one touch target to be activated at a time
/* =========================== */


/* === variables related to drawing graphs === */
uint16_t num_points = 10;
uint16_t x_interval = 50;
uint16_t y_interval = 50;
uint16_t x_scale = 100;
uint16_t y_scale = 100;
int16_t x_data[BUF_SIZE];
int16_t y_data[BUF_SIZE];
int start_idx = 0;
int end_idx = 10;
uint16_t test_points = 8;
int16_t x_test[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int16_t y_test[8] = {0, 10, 40, 35, 70, 11, 16, 28};
/* ==================================== */
/* ######################### GLOBAL VARIABLES - END ######################### */


/* ##################### FUNCTION DECLARATIONS - BEGIN ###################### */
static void touch_calibrate(void);

static void EVE_cmd_loadimages(void);
// static void EVE_cmd_bitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height, uint16_t x, uint16_t y);

static void EVE_cmd_statusbar_burst(void);
static void EVE_cmd_customclock_burst(void);
// static void EVE_cmd_wifi_status_burst(void);

static void increaseYear(Date* date);
static void decreaseYear(Date* date);
static void increaseMonth(Date* date);
static void decreaseMonth(Date* date);
static void increaseDay(Date* date);
static void decreaseDay(Date* date);

static void increaseHour(Date* date);
static void decreaseHour(Date* date);
static void increaseMinute(Date* date);
static void decreaseMinute(Date* date);
static void toggleSuffix(Date* date);

// static int16_t getMinValue(int16_t* arr, uint16_t num_points);
// static int16_t getMaxValue(int16_t* arr, uint16_t num_points);
// static void scaleData(int16_t* x_data, int16_t* y_data, uint16_t num_points);
static void EVE_cmd_display_graph_burst(int16_t* x_data, int16_t* y_data, uint16_t num_points);
static void EVE_cmd_demo_graph_burst(int16_t* x_data, int16_t* y_data, uint16_t num_points);

static void EVE_cmd_home_button_burst(uint8_t tag_value);
static void EVE_cmd_schedule_button_burst(uint8_t tag_value);
static void EVE_cmd_settings_button_burst(uint8_t tag_value);
static void EVE_cmd_data_button_burst(uint8_t tag_value);
static void EVE_cmd_demo_button_burst(uint8_t tag_value);
static void EVE_cmd_time_button_burst(uint8_t tag_value);
static void EVE_cmd_back_button_burst(uint8_t tag_value);

static void EVE_cmd_keypad_burst(void);
static void EVE_cmd_set_range_burst(void);
static void EVE_cmd_set_date_burst(void);

static void TFT_home(void);
static void TFT_data(void);
static void TFT_schedule(void);
static void TFT_settings(void);
static void TFT_demo(void);
static void TFT_time(void);
/* ###################### FUNCTION DECLARATIONS - END ####################### */


/* === functions for initialization === */
static void touch_calibrate(void) {
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

void TFT_init(void) {
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


/* === check for touch events and setup vars for TFT_display() === */
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
            case TAG_DEMO_BACKBUTTON:
            case TAG_SCHEDULE_BACKBUTTON:
            case TAG_SETTINGS_BACKBUTTON:
            case TAG_TIME_BACKBUTTON:
            case TAG_SETTINGS_DEMOBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_BUTTON;
                }
                break;
            case TAG_SETTINGS_TIMEBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_BUTTON;
                    new_date.year = current_date.year;
                    new_date.month = current_date.month;
                    new_date.day = current_date.day;
                    new_date.hour = current_date.hour;
                    new_date.minute = current_date.minute;
                    update_year_view(&new_date, &new_date_view);
                    update_month_view(&new_date, &new_date_view);
                    update_day_view(&new_date, &new_date_view);
                    update_hour_view(&new_date, &new_date_view);
                    update_minute_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_SETBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_BUTTON;
                    set_time(&new_date);    // set system time
                    update_time();          // update current_date
                    uint32_t datetime[5];
                    memcpy(datetime, &current_date, 5 * sizeof(uint32_t));
                    mem_erase(DATETIME_NAMESPACE);
                    mem_write(DATETIME_NAMESPACE, STORAGE_KEY, datetime, 5);
                }
                break;
            case TAG_SCHEDULE_KEY_0:
            case TAG_SCHEDULE_KEY_1:
            case TAG_SCHEDULE_KEY_2:
            case TAG_SCHEDULE_KEY_3:
            case TAG_SCHEDULE_KEY_4:
            case TAG_SCHEDULE_KEY_5:
            case TAG_SCHEDULE_KEY_6:
            case TAG_SCHEDULE_KEY_7:
            case TAG_SCHEDULE_KEY_8:
            case TAG_SCHEDULE_KEY_9:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    input[input_length] = INT_TO_ASCII(KEY_VALUE(tag));
                    input[input_length+1] = '\0';
                    input_length++;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_SCHEDULE_KEY_CLEAR:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    input_length = 0;
                    output_length = 0;
                    input[input_length] = '\0';
                    output[output_length] = '\0';
                }
                break;
            case TAG_SCHEDULE_KEY_ENTER:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    if (input_length > 0) {
                        uint32_t int_input[input_length];
                        for (int i = 0; i < input_length; i++) {
                            int_input[i] = ASCII_TO_INT(input[i]);
                        }
                        mem_write(SCHEDULE_NAMESPACE, STORAGE_KEY, int_input, input_length);
                        input_length = 0;
                        input[input_length] = '\0';
                    }
                }
                break;
            case TAG_SCHEDULE_READBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    uint32_t* int_output;
                    int_output = mem_read(SCHEDULE_NAMESPACE, STORAGE_KEY, &output_length);
                    for (int i = 0; i < output_length; i++) {
                        output[i] = INT_TO_ASCII(int_output[i]);
                    }
                    output[output_length] = '\0';
                    if (int_output != NULL) { free(int_output); }
                }
                break;
            case TAG_SETTINGS_ERASEBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    output_length = 0;
                    output[output_length] = '\0';
                    mem_erase(SCHEDULE_NAMESPACE);
                }
                break;
            case TAG_DATA_START_MONTH_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_START_MONTH_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_START_DAY_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_START_DAY_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_START_YEAR_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_START_YEAR_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_END_MONTH_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_END_MONTH_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_END_DAY_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_END_DAY_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_END_YEAR_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_END_YEAR_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
            case TAG_DATA_VIEWBUTTON:
            case TAG_TIME_MONTH_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    increaseMonth(&new_date);
                    update_month_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_MONTH_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    decreaseMonth(&new_date);
                    update_month_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_DAY_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    increaseDay(&new_date);
                    update_day_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_DAY_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    decreaseDay(&new_date);
                    update_day_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_YEAR_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    increaseYear(&new_date);
                    update_year_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_YEAR_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    decreaseYear(&new_date);
                    update_year_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_HOUR_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    increaseHour(&new_date);
                    update_hour_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_HOUR_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    decreaseHour(&new_date);
                    update_hour_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_MINUTE_UP:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    increaseMinute(&new_date);
                    update_minute_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_MINUTE_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    decreaseMinute(&new_date);
                    update_minute_view(&new_date, &new_date_view);
                }
                break;
            case TAG_TIME_AMPM_UP:
            case TAG_TIME_AMPM_DOWN:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                    toggleSuffix(&new_date);
                    update_hour_view(&new_date, &new_date_view);
                }
                break;
            case TAG_DEMO_WATERBUTTON:
            case TAG_DEMO_FOODBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = tag;
                    toggle_state[tag] = EVE_OPT_FLAT;
                    lock_delay = DELAY_KEY;
                }
                break;
        }
    }
}
/* =============================================================== */


/* === helper functions === */
static void increaseMonth(Date* date) {
    date->month = date->month == 12 ? 1 : date->month + 1;
}

static void decreaseMonth(Date* date) {
    date->month = date->month == 1 ? 12 : date->month - 1;
}

static void increaseYear(Date* date) {
    date->year += 1;
}

static void decreaseYear(Date* date) {
    date->year -= 1;
}

static void increaseDay(Date* date) {
    date->day = date->day == 30 ? 1 : date->day + 1;
}

static void decreaseDay(Date* date) {
    date->day = date->day == 1 ? 30 : date->day - 1;
}

static void increaseHour(Date* date) {
    date->hour = date->hour == 23 ? 0 : date->hour + 1;
}

static void decreaseHour(Date* date) {
    date->hour = date->hour == 0 ? 23 : date->hour - 1;
}

static void increaseMinute(Date* date) {
    date->minute = date->minute == 59 ? 0 : date->minute + 1;
}

static void decreaseMinute(Date* date) {
    date->minute = date->minute == 0 ? 59 : date->minute - 1;
}

static void toggleSuffix(Date* date) {
    uint16_t temp_hour = date->hour + 12;
    if (temp_hour > 23) {
        temp_hour -= 24;
    }
    date->hour = temp_hour;
}

// static int16_t getMinValue(int16_t* arr, uint16_t num_points) {
//    int16_t min_value = arr[0];
//     for (int i = 0; i < num_points; i++) {
//         min_value = arr[i] < min_value ? arr[i] : min_value;
//     }
//     return min_value;
// }

// static int16_t getMaxValue(int16_t* arr, uint16_t num_points) {
//     int16_t max_value = arr[0];
//     for (int i = 1; i < num_points; i++) {
//         max_value = arr[i] > max_value ? arr[i] : max_value;
//     }
//     return max_value;
// }

// static void scaleData(int16_t* x_data, int16_t* y_data, uint16_t num_points) {
//     int16_t y_max = getMaxValue(y_data, num_points);
//     return;
// }
/* ======================== */


/* === functions for displaying graphical elements on-screen === */
static void EVE_cmd_loadimages(void) {
    EVE_cmd_loadimage(MEM_PIC_WIFI, EVE_OPT_NODL, pic_wifi_32, sizeof(pic_wifi_32));
}

// static void EVE_cmd_bitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
//     EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
//     EVE_cmd_setbitmap_burst(addr, fmt, width, height);
//     EVE_cmd_dl_burst(VERTEX2F(x * 16, y * 16));
//     EVE_cmd_dl_burst(DL_END);
// }

static void EVE_cmd_customclock_burst(void) {
    update_time();
    update_view(&current_date, &current_date_view);

    EVE_color_rgb_burst(WHITE);
    EVE_cmd_fgcolor_burst(WHITE);
    EVE_cmd_text_burst(DATE_MONTH_X, DATE_Y, FONT_TIME, 0, current_date_view.month);
    EVE_cmd_text_burst(DATE_DAY_X, DATE_Y, FONT_TIME, 0, current_date_view.day);
    EVE_cmd_text_burst(DATE_YEAR_X, DATE_Y, FONT_TIME, 0, current_date_view.year);

    EVE_cmd_number_burst(CLOCK_X1, CLOCK_Y, FONT_TIME, 0, current_date_view.hour0);
    EVE_cmd_number_burst(CLOCK_X1 + DIGIT_WIDTH, CLOCK_Y, FONT_TIME, 0, current_date_view.hour1);
    EVE_cmd_text_burst(CLOCK_X2 - 5, CLOCK_Y - 1, FONT_TIME, 0, ":");
    EVE_cmd_number_burst(CLOCK_X2, CLOCK_Y, FONT_TIME, 0, current_date_view.minute0);
    EVE_cmd_number_burst(CLOCK_X2 + DIGIT_WIDTH, CLOCK_Y, FONT_TIME, 0, current_date_view.minute1);
    EVE_cmd_text_burst(CLOCK_X3, CLOCK_Y + 0, FONT_TIME, 0, current_date_view.suffix);
}

// static void EVE_cmd_wifi_status_burst(void) {
//     EVE_color_rgb_burst(WHITE);
//     EVE_cmd_bitmap_burst(MEM_PIC_WIFI, EVE_ARGB4, WIFI_WIDTH, WIFI_HEIGHT, WIFI_X, WIFI_Y);
// }

static void EVE_cmd_statusbar_burst(void) {
    // status bar background
    EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
    EVE_color_rgb_burst(BABY_BLUE);
    EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl_burst(VERTEX2F(0 * 16, 0 * 16));
    EVE_cmd_dl_burst(VERTEX2F(480 * 16, STATUS_BAR_HEIGHT * 16));

    // clock
    EVE_cmd_customclock_burst();
}

static void EVE_cmd_demo_graph_burst(int16_t* x_data, int16_t* y_data, uint16_t num_points) {
    EVE_color_rgb_burst(COLOR_RGB(100,100,100));
    EVE_cmd_dl_burst(LINE_WIDTH(8));

    // draw x-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, GRAPH_WATER_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + GRAPH_X_LEN) * 16, GRAPH_WATER_Y_BASE * 16));

    // draw y-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, GRAPH_WATER_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, (GRAPH_WATER_Y_BASE - 2 * GRAPH_Y_LEN) * 16));

    // draw horizontal gridlines
    EVE_color_rgb_burst(COLOR_RGB(220,220,220));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    for (int i = 1; i < 5; i++) {
        EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, (GRAPH_WATER_Y_BASE - i * y_interval) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + GRAPH_X_LEN) * 16, (GRAPH_WATER_Y_BASE - i * y_interval) * 16));
    }

    // draw vertical gridlines
    EVE_color_rgb_burst(COLOR_RGB(220,220,220));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    for (int i = 1; i < 5; i++) {
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + i * x_interval) * 16, GRAPH_WATER_Y_BASE * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + i * x_interval) * 16, (GRAPH_WATER_Y_BASE - 2 * GRAPH_Y_LEN) * 16));
    }

    // plot data
    EVE_color_rgb_burst(RED);
    EVE_cmd_dl_burst(LINE_WIDTH(12));
    for (int i = start_idx; i < end_idx; i++) {
        EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + x_data[i % BUF_SIZE]) * 16, (GRAPH_WATER_Y_BASE - y_data[i % BUF_SIZE]) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + x_data[(i+1) % BUF_SIZE]) * 16, (GRAPH_WATER_Y_BASE - y_data[(i+1) % BUF_SIZE]) * 16));
    }

    EVE_color_rgb_burst(BLUE);
    for (int i = start_idx; i < end_idx; i++) {
        EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + x_data[i % BUF_SIZE]) * 16, (GRAPH_WATER_Y_BASE - y_data[i % BUF_SIZE]) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + x_data[(i+1) % BUF_SIZE]) * 16, (GRAPH_WATER_Y_BASE - y_data[(i+1) % BUF_SIZE]) * 16));
    }
}

static void EVE_cmd_display_graph_burst(int16_t* x_data, int16_t* y_data, uint16_t num_points) {
    EVE_color_rgb_burst(COLOR_RGB(100,100,100));
    EVE_cmd_dl_burst(LINE_WIDTH(8));

    // draw food x-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_FOOD_X_BASE * 16, GRAPH_FOOD_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_FOOD_X_BASE + GRAPH_X_LEN) * 16, GRAPH_FOOD_Y_BASE * 16));

    // draw water x-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, GRAPH_WATER_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + GRAPH_X_LEN) * 16, GRAPH_WATER_Y_BASE * 16));

    // draw food y-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_FOOD_X_BASE * 16, GRAPH_FOOD_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_FOOD_X_BASE * 16, (GRAPH_FOOD_Y_BASE - GRAPH_Y_LEN) * 16));

    // draw water y-axis
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, GRAPH_WATER_Y_BASE * 16));
    EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, (GRAPH_WATER_Y_BASE - GRAPH_Y_LEN) * 16));

    // draw horizontal gridlines
    EVE_color_rgb_burst(COLOR_RGB(220,220,220));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    for (int i = 1; i < 3; i++) {
        EVE_cmd_dl_burst(VERTEX2F(GRAPH_FOOD_X_BASE * 16, (GRAPH_FOOD_Y_BASE - i * y_interval) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_FOOD_X_BASE + GRAPH_X_LEN) * 16, (GRAPH_FOOD_Y_BASE - i * y_interval) * 16));
        EVE_cmd_dl_burst(VERTEX2F(GRAPH_WATER_X_BASE * 16, (GRAPH_WATER_Y_BASE - i * y_interval) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + GRAPH_X_LEN) * 16, (GRAPH_WATER_Y_BASE - i * y_interval) * 16));
    }

    // draw vertical gridlines
    EVE_color_rgb_burst(COLOR_RGB(220,220,220));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    for (int i = 1; i < 5; i++) {
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_FOOD_X_BASE + i * x_interval) * 16, GRAPH_FOOD_Y_BASE * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_FOOD_X_BASE + i * x_interval) * 16, (GRAPH_FOOD_Y_BASE - GRAPH_Y_LEN) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + i * x_interval) * 16, GRAPH_WATER_Y_BASE * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + i * x_interval) * 16, (GRAPH_WATER_Y_BASE - GRAPH_Y_LEN) * 16));
    }

    // plot data
    EVE_color_rgb_burst(BLACK);
    EVE_cmd_dl_burst(LINE_WIDTH(12));
    for (int i = 0; i < (num_points - 1); i++) {
        EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_FOOD_X_BASE + x_data[i]) * 16, (GRAPH_FOOD_Y_BASE - y_data[i]) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_FOOD_X_BASE + x_data[i+1]) * 16, (GRAPH_FOOD_Y_BASE - y_data[i+1]) * 16));
        EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + x_data[i]) * 16, (GRAPH_WATER_Y_BASE - y_data[i]) * 16));
        EVE_cmd_dl_burst(VERTEX2F((GRAPH_WATER_X_BASE + x_data[i+1]) * 16, (GRAPH_WATER_Y_BASE - y_data[i+1]) * 16));
    }
}

static void EVE_cmd_keypad_burst(void) {
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_1);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_2);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_3);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_4);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_5);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_6);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_7);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_8);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_9);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_CLEAR);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_0);
    EVE_cmd_schedule_button_burst(TAG_SCHEDULE_KEY_ENTER);
}

static void EVE_cmd_set_range_burst(void) {
    EVE_color_rgb_burst(COLOR_RGB(100,100,100));
    EVE_cmd_text_burst(START_DATE_X, START_DATE_Y - DATE_HEIGHT - 40, FONT_DATE, 0, "START:");
    EVE_cmd_text_burst(END_DATE_X, END_DATE_Y - DATE_HEIGHT - 40, FONT_DATE, 0, "END:");

    EVE_color_rgb_burst(COLOR_RGB(230,230,230));
    EVE_cmd_dl_burst(LINE_WIDTH(5 * 16));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl_burst(VERTEX2F(START_DATE_X * 16, START_DATE_Y * 16));
    EVE_cmd_dl_burst(VERTEX2F((START_DATE_X + DATE_WIDTH) * 16, (START_DATE_Y + DATE_HEIGHT) * 16));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl_burst(VERTEX2F(END_DATE_X * 16, END_DATE_Y * 16));
    EVE_cmd_dl_burst(VERTEX2F((END_DATE_X + DATE_WIDTH) * 16, (END_DATE_Y + DATE_HEIGHT) * 16));

    EVE_color_rgb_burst(COLOR_RGB(80,80,80));
    EVE_cmd_text_burst(START_DATE_X, START_DATE_Y, FONT_DATE, 0, start_date_view.month);
    EVE_cmd_text_burst(START_DATE_X + 40, START_DATE_Y, FONT_DATE, 0, start_date_view.day);
    EVE_cmd_text_burst(START_DATE_X + 70, START_DATE_Y, FONT_DATE, 0, start_date_view.year);
    EVE_cmd_text_burst(END_DATE_X, END_DATE_Y, FONT_DATE, 0, end_date_view.month);
    EVE_cmd_text_burst(END_DATE_X + 40, END_DATE_Y, FONT_DATE, 0, end_date_view.day);
    EVE_cmd_text_burst(END_DATE_X + 70, END_DATE_Y, FONT_DATE, 0, end_date_view.year);
}

static void EVE_cmd_set_date_burst(void) {
    EVE_color_rgb_burst(COLOR_RGB(100,100,100));
    EVE_cmd_text_burst(140, 50, FONT_PRIMARY, 0, "Adjust date and time:");

    EVE_color_rgb_burst(COLOR_RGB(230,230,230));
    EVE_cmd_dl_burst(LINE_WIDTH(5 * 16));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl_burst(VERTEX2F(TIME_TEXT_X * 16, TIME_TEXT_Y * 16));
    EVE_cmd_dl_burst(VERTEX2F((TIME_TEXT_X + TIME_TEXT_WIDTH) * 16, (TIME_TEXT_Y + TIME_TEXT_HEIGHT) * 16));

    EVE_color_rgb_burst(COLOR_RGB(80,80,80));
    EVE_cmd_text_burst(TIME_X1, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.month);
    EVE_cmd_text_burst(TIME_X2, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.day);
    EVE_cmd_text_burst(TIME_X3, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.year);
    EVE_cmd_number_burst(TIME_X4, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.hour0);
    EVE_cmd_number_burst(TIME_X4 + 12, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.hour1);
    EVE_cmd_text_burst(TIME_X5 - 10, TIME_TEXT_Y - 2, FONT_PRIMARY, 0, ":");
    EVE_cmd_number_burst(TIME_X5, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.minute0);
    EVE_cmd_number_burst(TIME_X5 + 12, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.minute1);
    EVE_cmd_text_burst(TIME_X6, TIME_TEXT_Y, FONT_PRIMARY, 0, new_date_view.suffix);
}

void EVE_cmd_down_triangle_burst(uint16_t x, uint16_t y) {
    EVE_color_rgb_burst(WHITE);
    EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINE_STRIP);
    EVE_cmd_dl_burst(VERTEX2F((x + 8) * 16, (y + 6) * 16));
    EVE_cmd_dl_burst(VERTEX2F((x + 16) * 16, (y + 6) * 16));
    EVE_cmd_dl_burst(VERTEX2F((x + 12) * 16, (y + 12) * 16));
    EVE_cmd_dl_burst(VERTEX2F((x + 8) * 16, (y + 6) * 16));
}

void EVE_cmd_up_triangle_burst(uint16_t x, uint16_t y) {
    EVE_color_rgb_burst(WHITE);
    EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINE_STRIP);
    EVE_cmd_dl_burst(VERTEX2F((x + 8) * 16, (y + 12) * 16));
    EVE_cmd_dl_burst(VERTEX2F((x + 16) * 16, (y + 12) * 16));
    EVE_cmd_dl_burst(VERTEX2F((x + 12) * 16, (y + 6) * 16));
    EVE_cmd_dl_burst(VERTEX2F((x + 8) * 16, (y + 12) * 16));
}

static void EVE_cmd_home_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_HOME_DATABUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(HOME_BUTTON_X, HOME_BUTTON_Y, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "View data");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_HOME_SCHEDULEBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(HOME_BUTTON_X, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT + 10, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Schedule");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_HOME_SETTINGSBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(HOME_BUTTON_X, HOME_BUTTON_Y + 2 * (HOME_BUTTON_HEIGHT + 10), HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Settings");
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

static void EVE_cmd_schedule_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_SCHEDULE_KEY_1:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "1");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_2:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "2");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_3:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "3");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_4:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y + KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "4");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_5:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y + KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "5");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_6:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y + KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "6");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_7:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y + 2 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "7");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_8:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y + 2 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "8");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_9:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y + 2 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "9");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_CLEAR:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X, KEYPAD_Y + 3 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_KEY_WORD, toggle_state[tag_value], "Clear");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_0:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + KEY_X_OFFSET, KEYPAD_Y + 3 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "0");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_KEY_ENTER:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(KEYPAD_X + 2 * KEY_X_OFFSET, KEYPAD_Y + 3 * KEY_Y_OFFSET, KEY_WIDTH, KEY_HEIGHT, FONT_KEY_WORD, toggle_state[tag_value], "Enter");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SCHEDULE_READBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(80, 220, 200, 30, FONT_PRIMARY, toggle_state[tag_value], "Read from memory");
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

static void EVE_cmd_settings_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_SETTINGS_TIMEBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y, SETTINGS_BUTTON_WIDTH, SETTINGS_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Set date and time");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SETTINGS_DEMOBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y + SETTINGS_BUTTON_HEIGHT + 10, SETTINGS_BUTTON_WIDTH, SETTINGS_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "View demo");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_SETTINGS_ERASEBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y + 2 * (SETTINGS_BUTTON_HEIGHT + 10), SETTINGS_BUTTON_WIDTH, SETTINGS_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Reset flash");
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

static void EVE_cmd_data_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_DATA_START_MONTH_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X1, DATA_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(DATA_X1, DATA_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_START_MONTH_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X1, DATA_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(DATA_X1, DATA_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_START_DAY_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X2, DATA_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(DATA_X2, DATA_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_START_DAY_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X2, DATA_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(DATA_X2, DATA_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_START_YEAR_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X3, DATA_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(DATA_X3, DATA_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_START_YEAR_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X3, DATA_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(DATA_X3, DATA_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_END_MONTH_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X4, DATA_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(DATA_X4, DATA_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_END_MONTH_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X4, DATA_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(DATA_X4, DATA_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_END_DAY_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X5, DATA_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(DATA_X5, DATA_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_END_DAY_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X5, DATA_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(DATA_X5, DATA_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_END_YEAR_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X6, DATA_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(DATA_X6, DATA_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_END_YEAR_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(DATA_X6, DATA_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(DATA_X6, DATA_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_VIEWBUTTON:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_button_burst(DATA_VIEWBUTTON_X, DATA_VIEWBUTTON_Y, DATA_VIEWBUTTON_WIDTH, DATA_VIEWBUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "View data");
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

static void EVE_cmd_demo_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_DEMO_WATERBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(DEMO_BUTTON_X, DEMO_BUTTON_Y, DEMO_BUTTON_WIDTH, DEMO_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Dispense water");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DEMO_FOODBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(DEMO_BUTTON_X, DEMO_BUTTON_Y + DEMO_BUTTON_HEIGHT + 20, DEMO_BUTTON_WIDTH, DEMO_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Dispense food");
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

static void EVE_cmd_time_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_TIME_MONTH_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X1 + 5, TIME_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(TIME_X1 + 5, TIME_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_MONTH_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X1 + 5, TIME_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(TIME_X1 + 5, TIME_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_DAY_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X2, TIME_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(TIME_X2, TIME_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_DAY_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X2, TIME_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(TIME_X2, TIME_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_YEAR_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X3 + 12, TIME_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(TIME_X3 + 12, TIME_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_YEAR_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X3 + 12, TIME_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(TIME_X3 + 12, TIME_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_HOUR_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X4, TIME_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(TIME_X4, TIME_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_HOUR_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X4, TIME_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(TIME_X4, TIME_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_MINUTE_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X5, TIME_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(TIME_X5, TIME_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_MINUTE_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X5, TIME_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(TIME_X5, TIME_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_AMPM_UP:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X6 + 3, TIME_Y1, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_up_triangle_burst(TIME_X6 + 3, TIME_Y1);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_AMPM_DOWN:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_button_burst(TIME_X6 + 3, TIME_Y2, DATA_DATE_BUTTON_WIDTH, DATA_DATE_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_down_triangle_burst(TIME_X6 + 3, TIME_Y2);
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_TIME_12HR_TOGGLE:
            break;
        case TAG_TIME_SETBUTTON:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_button_burst(TIME_SETBUTTON_X, TIME_SETBUTTON_Y, TIME_SETBUTTON_WIDTH, TIME_SETBUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], "Save");
            EVE_cmd_dl_burst(TAG(0));
            break;
    }
}

static void EVE_cmd_back_button_burst(uint8_t tag_value) {
    switch (tag_value) {
        case TAG_DEMO_BACKBUTTON:
        case TAG_DATA_BACKBUTTON:
        case TAG_SCHEDULE_BACKBUTTON:
        case TAG_SETTINGS_BACKBUTTON:
        case TAG_TIME_BACKBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(BACK_BUTTON_X, BACK_BUTTON_Y, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, FONT_PRIMARY, toggle_state[tag_value], " ");
            EVE_cmd_dl_burst(TAG(0));
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
            EVE_cmd_dl_burst(DL_BEGIN | EVE_LINE_STRIP);
            EVE_cmd_dl_burst(VERTEX2F(32 * 16, 236 * 16));
            EVE_cmd_dl_burst(VERTEX2F(36 * 16, 232 * 16));
            EVE_cmd_dl_burst(VERTEX2F(36 * 16, 240 * 16));
            EVE_cmd_dl_burst(VERTEX2F(32 * 16, 236 * 16));
            break;
    }
}
/* ============================================================= */


/* === functions for each screen === */
static void TFT_home(void) {
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

        EVE_cmd_home_button_burst(TAG_HOME_DATABUTTON);
        EVE_cmd_home_button_burst(TAG_HOME_SCHEDULEBUTTON);
        EVE_cmd_home_button_burst(TAG_HOME_SETTINGSBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

static void TFT_data(void) {
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
            else if (toggle_state[TAG_DATA_START_MONTH_UP] != 0)    { toggle_state[TAG_DATA_START_MONTH_UP] = 0; }
            else if (toggle_state[TAG_DATA_START_MONTH_DOWN] != 0)  { toggle_state[TAG_DATA_START_MONTH_DOWN] = 0; }
            else if (toggle_state[TAG_DATA_START_DAY_UP] != 0)      { toggle_state[TAG_DATA_START_DAY_UP] = 0; }
            else if (toggle_state[TAG_DATA_START_DAY_DOWN] != 0)    { toggle_state[TAG_DATA_START_DAY_DOWN] = 0; }
            else if (toggle_state[TAG_DATA_START_YEAR_UP] != 0)     { toggle_state[TAG_DATA_START_YEAR_UP] = 0; }
            else if (toggle_state[TAG_DATA_START_YEAR_DOWN] != 0)   { toggle_state[TAG_DATA_START_YEAR_DOWN] = 0; }
            else if (toggle_state[TAG_DATA_END_MONTH_UP] != 0)      { toggle_state[TAG_DATA_END_MONTH_UP] = 0; }
            else if (toggle_state[TAG_DATA_END_MONTH_DOWN] != 0)    { toggle_state[TAG_DATA_END_MONTH_DOWN] = 0; }
            else if (toggle_state[TAG_DATA_END_DAY_UP] != 0)        { toggle_state[TAG_DATA_END_DAY_UP] = 0; }
            else if (toggle_state[TAG_DATA_END_DAY_DOWN] != 0)      { toggle_state[TAG_DATA_END_DAY_DOWN] = 0; }
            else if (toggle_state[TAG_DATA_END_YEAR_UP] != 0)       { toggle_state[TAG_DATA_END_YEAR_UP] = 0; }
            else if (toggle_state[TAG_DATA_END_YEAR_DOWN] != 0)     { toggle_state[TAG_DATA_END_YEAR_DOWN] = 0; }
            else if (toggle_state[TAG_DATA_VIEWBUTTON] != 0)        { toggle_state[TAG_DATA_VIEWBUTTON] = 0; }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();
        EVE_cmd_back_button_burst(TAG_DATA_BACKBUTTON);

        EVE_cmd_set_range_burst();

        EVE_cmd_display_graph_burst(x_test, y_test, test_points);

        EVE_cmd_data_button_burst(TAG_DATA_START_MONTH_UP);
        EVE_cmd_data_button_burst(TAG_DATA_START_MONTH_DOWN);
        EVE_cmd_data_button_burst(TAG_DATA_START_DAY_UP);
        EVE_cmd_data_button_burst(TAG_DATA_START_DAY_DOWN);
        EVE_cmd_data_button_burst(TAG_DATA_START_YEAR_UP);
        EVE_cmd_data_button_burst(TAG_DATA_START_YEAR_DOWN);
        EVE_cmd_data_button_burst(TAG_DATA_END_MONTH_UP);
        EVE_cmd_data_button_burst(TAG_DATA_END_MONTH_DOWN);
        EVE_cmd_data_button_burst(TAG_DATA_END_DAY_UP);
        EVE_cmd_data_button_burst(TAG_DATA_END_DAY_DOWN);
        EVE_cmd_data_button_burst(TAG_DATA_END_YEAR_UP);
        EVE_cmd_data_button_burst(TAG_DATA_END_YEAR_DOWN);

        EVE_cmd_data_button_burst(TAG_DATA_VIEWBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

static void TFT_schedule(void) {
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
            else if (toggle_state[TAG_SCHEDULE_KEY_1] != 0) { toggle_state[TAG_SCHEDULE_KEY_1] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_2] != 0) { toggle_state[TAG_SCHEDULE_KEY_2] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_3] != 0) { toggle_state[TAG_SCHEDULE_KEY_3] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_4] != 0) { toggle_state[TAG_SCHEDULE_KEY_4] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_5] != 0) { toggle_state[TAG_SCHEDULE_KEY_5] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_6] != 0) { toggle_state[TAG_SCHEDULE_KEY_6] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_7] != 0) { toggle_state[TAG_SCHEDULE_KEY_7] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_8] != 0) { toggle_state[TAG_SCHEDULE_KEY_8] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_9] != 0) { toggle_state[TAG_SCHEDULE_KEY_9] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_CLEAR] != 0) { toggle_state[TAG_SCHEDULE_KEY_CLEAR] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_0] != 0) { toggle_state[TAG_SCHEDULE_KEY_0] = 0; }
            else if (toggle_state[TAG_SCHEDULE_KEY_ENTER] != 0) { toggle_state[TAG_SCHEDULE_KEY_ENTER] = 0; }
            else if (toggle_state[TAG_SCHEDULE_READBUTTON] != 0) { toggle_state[TAG_SCHEDULE_READBUTTON] = 0; }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();
        EVE_cmd_back_button_burst(TAG_SCHEDULE_BACKBUTTON);

        EVE_cmd_keypad_burst();
        EVE_cmd_schedule_button_burst(TAG_SCHEDULE_READBUTTON);

        EVE_color_rgb_burst(BLACK);
        EVE_cmd_text_burst(20, 50, FONT_PRIMARY, 0, input);
        EVE_cmd_text_burst(20, 100, FONT_PRIMARY, 0, output);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

static void TFT_settings(void) {
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
            else if (toggle_state[TAG_SETTINGS_DEMOBUTTON] != 0) {
                screen_state = SCREENSTATE_DEMO;
                toggle_state[TAG_SETTINGS_DEMOBUTTON] = 0;
            }
            else if (toggle_state[TAG_SETTINGS_TIMEBUTTON] != 0) { 
                screen_state = SCREENSTATE_TIME;
                toggle_state[TAG_SETTINGS_TIMEBUTTON] = 0; 
            }
            else if (toggle_state[TAG_SETTINGS_ERASEBUTTON] != 0) { toggle_state[TAG_SETTINGS_ERASEBUTTON] = 0; }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();
        EVE_cmd_back_button_burst(TAG_SETTINGS_BACKBUTTON);

        EVE_cmd_settings_button_burst(TAG_SETTINGS_ERASEBUTTON);
        EVE_cmd_settings_button_burst(TAG_SETTINGS_TIMEBUTTON);
        EVE_cmd_settings_button_burst(TAG_SETTINGS_DEMOBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

static void TFT_time(void) {
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
            if (toggle_state[TAG_TIME_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_SETTINGS;
                toggle_state[TAG_TIME_BACKBUTTON] = 0;
            }
            else if (toggle_state[TAG_TIME_SETBUTTON] != 0) {
                screen_state = SCREENSTATE_SETTINGS;
                toggle_state[TAG_TIME_SETBUTTON] = 0;
            }
            else if (toggle_state[TAG_TIME_MONTH_UP] != 0) { toggle_state[TAG_TIME_MONTH_UP] = 0; }
            else if (toggle_state[TAG_TIME_MONTH_DOWN] != 0) { toggle_state[TAG_TIME_MONTH_DOWN] = 0; }
            else if (toggle_state[TAG_TIME_DAY_UP] != 0) { toggle_state[TAG_TIME_DAY_UP] = 0; }
            else if (toggle_state[TAG_TIME_DAY_DOWN] != 0) { toggle_state[TAG_TIME_DAY_DOWN] = 0; }
            else if (toggle_state[TAG_TIME_YEAR_UP] != 0) { toggle_state[TAG_TIME_YEAR_UP] = 0; }
            else if (toggle_state[TAG_TIME_YEAR_DOWN] != 0) { toggle_state[TAG_TIME_YEAR_DOWN] = 0; }
            else if (toggle_state[TAG_TIME_HOUR_UP] != 0) { toggle_state[TAG_TIME_HOUR_UP] = 0; }
            else if (toggle_state[TAG_TIME_HOUR_DOWN] != 0) { toggle_state[TAG_TIME_HOUR_DOWN] = 0; }
            else if (toggle_state[TAG_TIME_MINUTE_UP] != 0) { toggle_state[TAG_TIME_MINUTE_UP] = 0; }
            else if (toggle_state[TAG_TIME_MINUTE_DOWN] != 0) { toggle_state[TAG_TIME_MINUTE_DOWN] = 0; }
            else if (toggle_state[TAG_TIME_AMPM_UP] != 0) { toggle_state[TAG_TIME_AMPM_UP] = 0; }
            else if (toggle_state[TAG_TIME_AMPM_DOWN] != 0) { toggle_state[TAG_TIME_AMPM_DOWN] = 0; }
            else if (toggle_state[TAG_TIME_12HR_TOGGLE] != 0) { toggle_state[TAG_TIME_12HR_TOGGLE] = 0; }
            lock_delay = 0;
        }

        // status bar background
        EVE_cmd_dl_burst(LINE_WIDTH(2 * 16));
        EVE_color_rgb_burst(BABY_BLUE);
        EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
        EVE_cmd_dl_burst(VERTEX2F(0 * 16, 0 * 16));
        EVE_cmd_dl_burst(VERTEX2F(480 * 16, STATUS_BAR_HEIGHT * 16));
        EVE_cmd_back_button_burst(TAG_TIME_BACKBUTTON);

        EVE_cmd_set_date_burst();
        EVE_cmd_time_button_burst(TAG_TIME_MONTH_UP);
        EVE_cmd_time_button_burst(TAG_TIME_MONTH_DOWN);
        EVE_cmd_time_button_burst(TAG_TIME_DAY_UP);
        EVE_cmd_time_button_burst(TAG_TIME_DAY_DOWN);
        EVE_cmd_time_button_burst(TAG_TIME_YEAR_UP);
        EVE_cmd_time_button_burst(TAG_TIME_YEAR_DOWN);
        EVE_cmd_time_button_burst(TAG_TIME_HOUR_UP);
        EVE_cmd_time_button_burst(TAG_TIME_HOUR_DOWN);
        EVE_cmd_time_button_burst(TAG_TIME_MINUTE_UP);
        EVE_cmd_time_button_burst(TAG_TIME_MINUTE_DOWN);
        EVE_cmd_time_button_burst(TAG_TIME_AMPM_UP);
        EVE_cmd_time_button_burst(TAG_TIME_AMPM_DOWN);
        EVE_cmd_time_button_burst(TAG_TIME_12HR_TOGGLE);
        EVE_cmd_time_button_burst(TAG_TIME_SETBUTTON);


        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

static void TFT_demo(void) {
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
            if (toggle_state[TAG_DEMO_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_SETTINGS;
                toggle_state[TAG_DEMO_BACKBUTTON] = 0;
            }
            else if (toggle_state[TAG_DEMO_WATERBUTTON] != 0) { toggle_state[TAG_DEMO_WATERBUTTON] = 0; }
            else if (toggle_state[TAG_DEMO_FOODBUTTON] != 0) { toggle_state[TAG_DEMO_FOODBUTTON] = 0; }
            lock_delay = 0;
        }

        EVE_cmd_statusbar_burst();
        EVE_cmd_back_button_burst(TAG_DEMO_BACKBUTTON);

        EVE_cmd_demo_graph_burst(x_data, y_data, 0);

        EVE_cmd_demo_button_burst(TAG_DEMO_WATERBUTTON);
        EVE_cmd_demo_button_burst(TAG_DEMO_FOODBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
    }
}

/* ================================= */


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
        case SCREENSTATE_DEMO:
            TFT_demo();
            break;
        case SCREENSTATE_TIME:
            TFT_time();
            break;
    }
}

/* TEMPLATE FOR NEW SCREENS */
// void TFT_screen(void) {
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
//     }
// }