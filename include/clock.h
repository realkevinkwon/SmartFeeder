#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma once

#include <stdint.h>
#include "esp_sntp.h"

typedef struct _Time {
    uint8_t hour0;
    uint8_t hour1;
    uint8_t minute0;
    uint8_t minute1;
    char suffix[3];
} Time;

extern Time current_time;

void update_time(void);
void set_timezone(void);

#endif /* __CLOCK_H__ */