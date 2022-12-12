#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <nvs.h>
#include <nvs_flash.h>

#define SETTINGS_NAMESPACE "settings"       // general system settings
#define DATETIME_NAMESPACE "datetime"       // time and date
#define SCHEDULE_NAMESPACE "schedule"       // saved feeding schedule
#define WATER_NAMESPACE "water"             // water consumption data
#define FOOD_NAMESPACE "food"               // food consumption data
#define STORAGE_KEY "key"

typedef struct _FeedingTime {
    uint32_t hour;
    uint32_t minute;
    uint32_t food_amount;
    uint32_t water_amount;
} FeedingTime;

void memory_init(void);

void mem_write(const char* namespace, const char* key, uint32_t* new_data, size_t length);

// data is malloced within function and must freed from calling function
uint32_t* mem_read(const char* namespace, const char* key, size_t* length);

void mem_erase(const char* namespace);

#endif /* __MEMORY_H__ */