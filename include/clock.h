#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma once

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "memory.h"

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

extern Date current_date;
extern Date start_date;
extern Date end_date;
extern Date new_date;
extern DateView current_date_view;
extern DateView start_date_view;
extern DateView end_date_view;
extern DateView new_date_view;

void clock_init(void);
void set_timezone(const char* new_timezone);
void update_time(void);
void update_view(Date* date, DateView* date_view);
void set_time(Date* date);

// takes information from date and places into date_view
void update_year_view(Date* date, DateView* date_view);
void update_month_view(Date* date, DateView* date_view);
void update_day_view(Date* date, DateView* date_view);
void update_hour_view(Date* date, DateView* date_view);
void update_minute_view(Date* date, DateView* date_view);
void update_suffix_view(Date* date, DateView* date_view);

#endif /* __CLOCK_H__ */