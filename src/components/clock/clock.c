#include "clock.h"

time_t raw_time;
struct tm time_info;
Time current_time = {
    .hour0 = 0,
    .hour1 = 0,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'}
};
char* time_zone = "EST5EDT,M3.2.0,M11.1.0";

void update_time(void) {
    // get the new raw time (time_t) and translate to time_info (struct tm)
    time(&raw_time);
    localtime_r(&raw_time, &time_info);

    uint8_t temp_hour = time_info.tm_hour;
    uint8_t temp_minute = time_info.tm_min;

    // translate 24-hour time to 12-hour time
    if (temp_hour >= 0 && temp_hour <= 12) {
        if (temp_hour == 0) {
            temp_hour = 12;
        }
        current_time.suffix[0] = 'A';
    }
    else {
        temp_hour -= 12;
        current_time.suffix[0] = 'P';
    }

    current_time.hour0 = temp_hour / 10;
    current_time.hour1 = temp_hour - 10 * current_time.hour0;
    current_time.minute0 = temp_minute / 10;
	current_time.minute1 = temp_minute - 10 * current_time.minute0;
    current_time.suffix[1] = 'M';
    current_time.suffix[2] = '\0';
}