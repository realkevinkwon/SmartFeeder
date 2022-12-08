#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma once

#include <stdint.h>
#include <time.h>

typedef struct _Time {
    uint8_t hour0;
    uint8_t hour1;
    uint8_t minute0;
    uint8_t minute1;
    char suffix[3];
} Time;

extern Time current_time;

void clock_init(void);
void set_timezone(const char* new_timezone);
void update_time(void);

#endif /* __CLOCK_H__ */