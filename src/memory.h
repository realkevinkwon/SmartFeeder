#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <nvs.h>
#include <nvs_flash.h>

void mem_init(void);

void mem_write(const char* namespace, uint32_t* data);

// data is malloced within function and must freed from calling function
void mem_read(const char* namespace, uint32_t* data);

#endif /* __MEMORY_H__ */