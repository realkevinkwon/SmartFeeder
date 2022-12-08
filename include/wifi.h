#ifndef __WIFI_H__
#define __WIFI_H__

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

extern time_t raw_time;
extern struct tm* time_info;
extern Time current_time;

void wifi_init(void); 
void update_time(void);

#endif /* __WIFI_H__ */