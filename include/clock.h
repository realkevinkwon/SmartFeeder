#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma once

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "memory.h"


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