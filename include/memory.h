#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <nvs.h>
#include <nvs_flash.h>

#define STORAGE_NAMESPACE "storage"
#define SETTINGS_NAMESPACE "settings"       // general system settings
#define DATETIME_NAMESPACE "datetime"       // time and date
#define SCHEDULE_NAMESPACE "schedule"       // saved feeding schedule
#define WATER_NAMESPACE "water"             // water consumption data
#define FOOD_NAMESPACE "food"               // food consumption data

void memory_init(void);

void mem_write(const char* namespace, uint32_t* new_data, size_t length);

// data is malloced within function and must freed from calling function
uint32_t* mem_read(const char* namespace, size_t* length);

void mem_erase(void);

#endif /* __MEMORY_H__ */