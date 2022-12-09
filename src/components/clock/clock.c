#include "clock.h"

#define TIMEZONE_DEFAULT "EST5EDT,M3.2.0,M11.1.0"

time_t raw_time;
struct timeval tv = { .tv_sec = 100000, .tv_usec = 0 };
struct tm time_info;
Time current_time = {
    .hour0 = 0,
    .hour1 = 0,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'}
};

void clock_init(void) {
    set_timezone(TIMEZONE_DEFAULT);
}

void set_timezone(const char* timezone) {
    setenv("TZ", timezone, 1);
    tzset();
}

void set_time(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute, char* suffix) {
    if (hour == 12 && suffix[0] == 'A') {
        hour = 0;
    }
    else if (suffix[0] == 'P') {
        hour += 12;
    }

    time_info = (struct tm) {
        .tm_sec = 0,
        .tm_min = minute,
        .tm_hour = hour,
        .tm_mday = day,
        .tm_mon = month - 1,
        .tm_year = year,
    };

    tv.tv_sec = mktime(&time_info);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}

void update_month_str(char* month_str, uint16_t month) {
    switch (month) {
        case 1:
            month_str[0] = 'J';
            month_str[1] = 'A';
            month_str[2] = 'N';
            break;
        case 2:
            month_str[0] = 'F';
            month_str[1] = 'E';
            month_str[2] = 'B';
            break;
        case 3:
            month_str[0] = 'M';
            month_str[1] = 'A';
            month_str[2] = 'R';
            break;
        case 4:
            month_str[0] = 'A';
            month_str[1] = 'P';
            month_str[2] = 'R';
            break;
        case 5:
            month_str[0] = 'M';
            month_str[1] = 'A';
            month_str[2] = 'Y';
            break;
        case 6:
            month_str[0] = 'J';
            month_str[1] = 'U';
            month_str[2] = 'N';
            break;
        case 7:
            month_str[0] = 'J';
            month_str[1] = 'U';
            month_str[2] = 'L';
            break;
        case 8:
            month_str[0] = 'A';
            month_str[1] = 'U';
            month_str[2] = 'G';
            break;
        case 9:
            month_str[0] = 'S';
            month_str[1] = 'E';
            month_str[2] = 'P';
            break;
        case 10:
            month_str[0] = 'O';
            month_str[1] = 'C';
            month_str[2] = 'T';
            break;
        case 11:
            month_str[0] = 'N';
            month_str[1] = 'O';
            month_str[2] = 'V';
            break;
        case 12:
            month_str[0] = 'D';
            month_str[1] = 'E';
            month_str[2] = 'C';
            break;
    }
    month_str[3] = '\0';
}

void update_day_str(char* day_str, uint16_t day) {
    uint8_t temp_day = day / 10;
    day_str[0] = temp_day;
    day_str[1] = day - temp_day * 10;
    day_str[2] = '\0';
}

void update_year_str(char* year_str, uint16_t year) {
    uint8_t temp_year = year / 1000;
    year_str[0] = temp_year;
    year -= temp_year * 1000;

    temp_year = year / 100;
    year_str[1] = temp_year;
    year -= temp_year * 100;

    temp_year = year / 10;
    year_str[2] = temp_year;
    year_str[3] = year - temp_year * 10;
    year_str[4] = '\0';
}

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