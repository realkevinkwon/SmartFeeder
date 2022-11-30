#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <nvs.h>
#include <nvs_flash.h>

void mem_init(void);

void mem_write(void);

void mem_read(void);

#endif /* __MEMORY_H__ */