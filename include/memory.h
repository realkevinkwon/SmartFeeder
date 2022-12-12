#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <string.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"

#define SETTINGS_NAMESPACE "settings"       // general system settings
#define DATETIME_NAMESPACE "datetime"       // time and date
#define SCHEDULE_NAMESPACE "schedule"       // saved feeding schedule
#define WATER_NAMESPACE "water"             // water consumption data
#define FOOD_NAMESPACE "food"               // food consumption data
#define STORAGE_KEY "key"

typedef struct _DateView {
    char year[5];
    char month[4];
    char day[3];
    uint8_t hour0;
    uint8_t hour1;
    uint8_t minute0;
    uint8_t minute1;
    char suffix[3];
} DateView;

typedef struct _Date {
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
} Date;

typedef struct _FeedingTime {
    uint32_t hour;
    uint32_t minute;
    uint32_t food_amount;
    uint32_t water_amount;
} FeedingTime;

typedef struct _Schedule {
    FeedingTime feeding_times[5];
} Schedule;

extern Schedule schedule;
extern uint32_t food_amount;
extern uint32_t water_amount;
extern Date current_date;
extern Date start_date;
extern Date end_date;
extern Date new_date;
extern Date feed_date;
extern DateView current_date_view;
extern DateView start_date_view;
extern DateView end_date_view;
extern DateView new_date_view;
extern DateView feed_date_view;
extern uint8_t feed_select;
extern uint8_t feed_toggle[5];

void select_feeding_time(FeedingTime* feeding_time, DateView* feed_date_view);
void memory_init(void);

void mem_write(const char* namespace, const char* key, uint32_t* new_data, size_t length);

// data is malloced within function and must freed from calling function
uint32_t* mem_read(const char* namespace, const char* key, size_t* length);

void mem_erase(const char* namespace);

#endif /* __MEMORY_H__ */