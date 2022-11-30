#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <nvs.h>
#include <nvs_flash.h>

void mem_init(void);

void mem_write(const char* namespace, uint32_t* new_data, size_t length);

// data is malloced within function and must freed from calling function
uint32_t* mem_read(const char* namespace, size_t* length);

#endif /* __MEMORY_H__ */