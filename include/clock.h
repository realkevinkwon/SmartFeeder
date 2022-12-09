#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma once

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

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
void set_time(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute, char* suffix);

void update_month_str(char* month_str, uint16_t month);
void update_day_str(char* day_str, uint16_t day);
void update_year_str(char* year_str, uint16_t year);

#endif /* __CLOCK_H__ */