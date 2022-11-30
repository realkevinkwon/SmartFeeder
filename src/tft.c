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


#define TEST_UTF8 0


/* some pre-definded colors */
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


#define DISPLAY_ORIENTATION 0

#define LOCK_DELAY 10

#define SCREENSTATE_HOME 0
#define SCREENSTATE_DATA 1
#define SCREENSTATE_SCHEDULE 2
#define SCREENSTATE_SETTINGS 3

#define TAG_HOME_DATABUTTON 1
#define TAG_HOME_SCHEDULEBUTTON 2
#define TAG_HOME_SETTINGSBUTTON 3

#define TAG_DATA_BACKBUTTON 4

#define TAG_SCHEDULE_BACKBUTTON 5

#define TAG_SETTINGS_BACKBUTTON 6

#define MEM_DL_STATIC (EVE_RAM_G_SIZE - 4096) /* 0xff000 - start-address of the static part of the display-list, upper 4k of gfx-mem */

uint32_t num_dl_static; /* amount of bytes in the static part of our display-list */
uint8_t tft_active = 0;
uint16_t num_profile_a, num_profile_b;

uint16_t toggle_state[255];    // holds toggle states for various ui elements

#define LAYOUT_Y1 66


void touch_calibrate(void)
{

/* send pre-recorded touch calibration values, depending on the display the code is compiled for */

#if defined (EVE_CFAF240400C1_030SC)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x0000ed11);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00001139);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff76809);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000000);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010690);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xfffadf2e);
#endif

#if defined (EVE_CFAF320240F_035T)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00005614);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x0000009e);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff43422);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x0000001d);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0xffffbda4);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x00f8f2ef);
#endif

#if defined (EVE_CFAF480128A0_039TC)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00010485);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x0000017f);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfffb0bd3);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000073);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000e293);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x00069904);
#endif

#if defined (EVE_CFAF800480E0_050SC)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000107f9);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0xffffff8c);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff451ae);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x000000d2);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000feac);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xfffcfaaf);
#endif

#if defined (EVE_PAF90)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00000159);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x0001019c);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff93625);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00010157);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00000000);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x0000c101);
#endif

#if defined (EVE_RiTFT43)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000062cd);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0xfffffe45);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff45e0a);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x000001a3);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00005b33);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFFbb870);
#endif

#if defined (EVE_EVE2_38)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00007bed);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x000001b0);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff60aa5);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000095);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0xffffdcda);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x00829c08);
#endif

#if defined (EVE_EVE2_35G) ||  defined (EVE_EVE3_35G)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000109E4);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x000007A6);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFEC1EBA);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x0000072C);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x0001096A);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFF469CF);
#endif

#if defined (EVE_EVE2_43G) ||  defined (EVE_EVE3_43G)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x0000a1ff);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000680);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xffe54cc2);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0xffffff53);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000912c);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xfffe628d);
#endif

#if defined (EVE_EVE2_50G) || defined (EVE_EVE3_50G)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000109E4);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x000007A6);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFEC1EBA);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x0000072C);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x0001096A);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFF469CF);
#endif

#if defined (EVE_EVE2_70G)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000105BC);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0xFFFFFA8A);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0x00004670);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0xFFFFFF75);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010074);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFFF14C8);
#endif

#if defined (EVE_NHD_35)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x0000f78b);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000427);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfffcedf8);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0xfffffba4);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000f756);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x0009279e);
#endif

#if defined (EVE_RVT70)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000074df);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x000000e6);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xfffd5474);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x000001af);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00007e79);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xffe9a63c);
#endif

#if defined (EVE_FT811CB_HY50HD)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 66353);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 712);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 4293876677);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 4294966157);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 67516);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 418276);
#endif

#if defined (EVE_ADAM101)
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x000101E3);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000114);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFF5EEBA);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0xFFFFFF5E);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010226);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x0000C783);
#endif

/* activate this if you are using a module for the first time or if you need to re-calibrate it */
/* write down the numbers on the screen and either place them in one of the pre-defined blocks above or make a new block */
#if 1
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


void initStaticBackground(void)
{
    EVE_cmd_dl(CMD_DLSTART); /* Start the display list */

    EVE_cmd_dl(TAG(0)); /* do not use the following objects for touch-detection */

    EVE_cmd_bgcolor(0x00c0c0c0); /* light grey */

    EVE_cmd_dl(VERTEX_FORMAT(0)); /* reduce precision for VERTEX2F to 1 pixel instead of 1/16 pixel default */

    /* draw a rectangle on top */
    EVE_cmd_dl(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl(LINE_WIDTH(1*16)); /* size is in 1/16 pixel */

    EVE_cmd_dl(DL_COLOR_RGB | BABY_BLUE);
    EVE_cmd_dl(VERTEX2F(0,0));
    EVE_cmd_dl(VERTEX2F(EVE_HSIZE,LAYOUT_Y1-2));
    EVE_cmd_dl(DL_END);

    /* display the logo */
    // EVE_cmd_dl(DL_COLOR_RGB | WHITE);
    // EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    // EVE_cmd_setbitmap(MEM_LOGO, EVE_ARGB1555, 56, 56);
    // EVE_cmd_dl(VERTEX2F(EVE_HSIZE - 58, 5));
    // EVE_cmd_dl(DL_END);

    /* draw a black line to separate things */
    EVE_cmd_dl(DL_COLOR_RGB | BLACK);
    EVE_cmd_dl(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(0,LAYOUT_Y1-2));
    EVE_cmd_dl(VERTEX2F(EVE_HSIZE,LAYOUT_Y1-2));
    EVE_cmd_dl(DL_END);

#if (TEST_UTF8 != 0) && (EVE_GEN > 2)
    EVE_cmd_setfont2(12,MEM_FONT,32); /* assign bitmap handle to a custom font */
    EVE_cmd_text(EVE_HSIZE/2, 15, 12, EVE_OPT_CENTERX, "EVE Demo");
#else
    EVE_cmd_text(EVE_HSIZE/2, 15, 29, EVE_OPT_CENTERX, "EVE Demo");
#endif

    /* add the static text to the list */
#if defined (EVE_DMA)
    EVE_cmd_text(10, EVE_VSIZE - 65, 26, 0, "Bytes:");
#endif
    EVE_cmd_text(10, EVE_VSIZE - 50, 26, 0, "DL-size:");
    EVE_cmd_text(10, EVE_VSIZE - 35, 26, 0, "Time1:");
    EVE_cmd_text(10, EVE_VSIZE - 20, 26, 0, "Time2:");

    EVE_cmd_text(105, EVE_VSIZE - 35, 26, 0, "us");
    EVE_cmd_text(105, EVE_VSIZE - 20, 26, 0, "us");

    EVE_execute_cmd();

    num_dl_static = EVE_memRead16(REG_CMD_DL);

    EVE_cmd_memcpy(MEM_DL_STATIC, EVE_RAM_DL, num_dl_static);
    EVE_execute_cmd();
}

/* memory-map defines */
#define MEM_PIC_WIFI 0x000f0000

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

        // EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0 << 16);
        // EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 1 << 16);
        // EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0 << 16);
        // EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 1 << 16);
        // EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0 << 16);
        // EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0 << 16);

        // touch_calibrate();   // use this to get touch transform values

        EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00000CC1);
        EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x0000FE4c);
        EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFF6BA43);
        EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x000115DF);
        EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0xFFFFF849);
        EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFF8DE4B);


#if (TEST_UTF8 != 0) && (EVE_GEN > 2)   /* we need a BT81x for this */
    #if 0
        /* this is only needed once to transfer the flash-image to the external flash */
        uint32_t datasize;

        EVE_cmd_inflate(0, flash, sizeof(flash)); /* de-compress flash-image to RAM_G */
        datasize = EVE_cmd_getptr(); /* we unpacked to RAM_G address 0x0000, so the first address after the unpacked data also is the size */
        EVE_cmd_flashupdate(0,0,4096); /* write blob first */
        if (E_OK == EVE_init_flash())
        {
            EVE_cmd_flashupdate(0,0,(datasize|4095)+1); /* size must be a multiple of 4096, so set the lower 12 bits and add 1 */
        }
    #endif

    if (E_OK == EVE_init_flash())
    {
        EVE_cmd_flashread(MEM_FONT, 84928, 320); /* copy .xfont from FLASH to RAM_G, offset and length are from the .map file */
    }

#endif /* TEST_UTF8 */

        // EVE_cmd_inflate(MEM_LOGO, logo, sizeof(logo)); /* load logo into gfx-memory and de-compress it */
        EVE_cmd_loadimages();
        
        EVE_cmd_setrotate(DISPLAY_ORIENTATION);
    }
}

uint8_t screen_state = SCREENSTATE_HOME;                  // holds which screen should be displayed
uint16_t display_list_size = 0;
uint16_t lock_delay = 0;

uint8_t toggle_lock = 0;

/* check for touch events and setup vars for TFT_display() */
void TFT_touch(void)
{
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
                if (0 == toggle_lock) {
                    toggle_lock = TAG_HOME_DATABUTTON;
                    toggle_state[TAG_HOME_DATABUTTON] = EVE_OPT_FLAT;
                    lock_delay = LOCK_DELAY;
                }
                break;
            case TAG_HOME_SCHEDULEBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = TAG_HOME_SCHEDULEBUTTON;
                    toggle_state[TAG_HOME_SCHEDULEBUTTON] = EVE_OPT_FLAT;
                    lock_delay = LOCK_DELAY;
                }
                break;
            case TAG_HOME_SETTINGSBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = TAG_HOME_SETTINGSBUTTON;
                    toggle_state[TAG_HOME_SETTINGSBUTTON] = EVE_OPT_FLAT;
                    lock_delay = LOCK_DELAY;
                }
                break;
            case TAG_DATA_BACKBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = TAG_DATA_BACKBUTTON;
                    toggle_state[TAG_DATA_BACKBUTTON] = EVE_OPT_FLAT;
                    lock_delay = LOCK_DELAY;
                }
                break;
            case TAG_SCHEDULE_BACKBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = TAG_SCHEDULE_BACKBUTTON;
                    toggle_state[TAG_SCHEDULE_BACKBUTTON] = EVE_OPT_FLAT;
                    lock_delay = LOCK_DELAY;
                }
                break;
            case TAG_SETTINGS_BACKBUTTON:
                if (0 == toggle_lock) {
                    toggle_lock = TAG_SETTINGS_BACKBUTTON;
                    toggle_state[TAG_SETTINGS_BACKBUTTON] = EVE_OPT_FLAT;
                    lock_delay = LOCK_DELAY;
                }
                break;
        }
    }
}

#define HOME_BUTTON_WIDTH 110
#define HOME_BUTTON_HEIGHT 110
#define HOME_BUTTON_X 20
#define HOME_BUTTON_Y 60
#define DIGIT_WIDTH 16
#define CLOCK_X1 340
#define CLOCK_X2 CLOCK_X1 + 38
#define CLOCK_X3 CLOCK_X2 + 40
#define CLOCK_Y 2
#define MENU_FONT 28
#define TIME_FONT 30

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
    EVE_cmd_number_burst(CLOCK_X1, CLOCK_Y, TIME_FONT, 0, 0);
    EVE_cmd_number_burst(CLOCK_X1 + DIGIT_WIDTH, CLOCK_Y, TIME_FONT, 0, 9);
    EVE_cmd_text_burst(CLOCK_X2 - 5, CLOCK_Y + 1, 24, 0, ":");
    EVE_cmd_number_burst(CLOCK_X2, CLOCK_Y, TIME_FONT, 0, 2);
    EVE_cmd_number_burst(CLOCK_X2 + DIGIT_WIDTH, CLOCK_Y, TIME_FONT, 0, 8);
    EVE_cmd_text_burst(CLOCK_X3, CLOCK_Y + 0, TIME_FONT, 0, "PM");
}

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
#define GRAPH_X_INTERVAL GRAPH_X_SCALE * 50
#define GRAPH_Y_INTERVAL GRAPH_Y_SCALE * 50

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
            EVE_cmd_text_burst(HOME_BUTTON_X + 10, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT - 30, MENU_FONT, 0, "Data");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_HOME_SCHEDULEBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(2 * HOME_BUTTON_X + HOME_BUTTON_WIDTH, HOME_BUTTON_Y, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, MENU_FONT, toggle_state[tag_value], " ");
            EVE_cmd_text_burst(2 * HOME_BUTTON_X + HOME_BUTTON_WIDTH + 10, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT - 30, MENU_FONT, 0, "Schedule");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_HOME_SETTINGSBUTTON:
            EVE_cmd_fgcolor_burst(BABY_BLUE);
            EVE_color_rgb_burst(WHITE);
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(3 * HOME_BUTTON_X + 2 * HOME_BUTTON_WIDTH, HOME_BUTTON_Y, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, MENU_FONT, toggle_state[tag_value], " ");
            EVE_cmd_text_burst(3 * HOME_BUTTON_X + 2 * HOME_BUTTON_WIDTH + 10, HOME_BUTTON_Y + HOME_BUTTON_HEIGHT - 30, MENU_FONT, 0, "Settings");
            EVE_cmd_dl_burst(TAG(0));
            break;
        case TAG_DATA_BACKBUTTON:
        case TAG_SCHEDULE_BACKBUTTON:
        case TAG_SETTINGS_BACKBUTTON:
            EVE_cmd_dl_burst(TAG(tag_value));
            EVE_cmd_button_burst(20, 220, 30, 30, MENU_FONT, toggle_state[tag_value], " ");
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

void TFT_home(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay == 1) {
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
        else if (lock_delay != 0) {
            lock_delay--;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_custombutton_burst(TAG_HOME_DATABUTTON);

        EVE_cmd_custombutton_burst(TAG_HOME_SCHEDULEBUTTON);

        EVE_cmd_custombutton_burst(TAG_HOME_SETTINGSBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
        while (EVE_busy());
    }
}

uint16_t num_points = 8;
/* 
    data MUST be in order:
    x_data = {x_0, x_1, x_2,...,x_i, x_i+1,...}
    y_data = {y_0, y_1, y_2,...,y_i, y_i+1,...}
*/
int16_t x_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int16_t y_data[8] = {0, 10, 40, 35, 70, 11, 16, 28};

void TFT_data(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay == 1) {
            if (toggle_state[TAG_DATA_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_HOME;
                toggle_state[TAG_DATA_BACKBUTTON] = 0;
            }
            lock_delay = 0;
        }
        else if (lock_delay != 0) {
            lock_delay--;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_display_graph_burst(x_data, y_data, num_points);

        EVE_cmd_custombutton_burst(TAG_DATA_BACKBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
        while (EVE_busy());
    }
}

void TFT_schedule(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay == 1) {
            if (toggle_state[TAG_SCHEDULE_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_HOME;
                toggle_state[TAG_SCHEDULE_BACKBUTTON] = 0;
            }
            lock_delay = 0;
        }
        else if (lock_delay != 0) {
            lock_delay--;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_custombutton_burst(TAG_SCHEDULE_BACKBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
        while (EVE_busy());
    }
}

void TFT_settings(void) {
    if (tft_active != 0) {
        EVE_start_cmd_burst();
        EVE_cmd_dl_burst(CMD_DLSTART);
        EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE);
        EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
        EVE_cmd_dl_burst(TAG(0));

        if (lock_delay == 1) {
            if (toggle_state[TAG_SETTINGS_BACKBUTTON] != 0) {
                screen_state = SCREENSTATE_HOME;
                toggle_state[TAG_SETTINGS_BACKBUTTON] = 0;
            }
            lock_delay = 0;
        }
        else if (lock_delay != 0) {
            lock_delay--;
        }

        EVE_cmd_statusbar_burst();

        EVE_cmd_custombutton_burst(TAG_SETTINGS_BACKBUTTON);

        EVE_cmd_dl_burst(DL_DISPLAY);
        EVE_cmd_dl_burst(CMD_SWAP);
        EVE_end_cmd_burst();
        while (EVE_busy());
    }
}

/*
    dynamic portion of display-handling, meant to be called every 20ms or more
*/
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


//         EVE_cmd_dl_burst(DL_DISPLAY);
//         EVE_cmd_dl_burst(CMD_SWAP);
//         EVE_end_cmd_burst();
//         while (EVE_busy());
//     }
// }