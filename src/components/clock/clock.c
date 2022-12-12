#include "clock.h"
#include <stdio.h>

#define TIMEZONE_DEFAULT "EST5EDT,M3.2.0,M11.1.0"
#define INT_TO_ASCII(x) (x + 48)

time_t raw_time;
struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
struct tm time_info;

Date current_date = {
    .year = 2022,
    .month = 1,
    .day = 1,
    .hour = 0,
    .minute = 0,
};
Date start_date = {
    .year = 2022,
    .month = 1,
    .day = 1,
    .hour = 0,
    .minute = 0,
};
Date end_date = {
    .year = 2022,
    .month = 1,
    .day = 1,
    .hour = 0,
    .minute = 0,
};
Date new_date = {
    .year = 2022,
    .month = 1,
    .day = 1,
    .hour = 0,
    .minute = 0,
};

DateView current_date_view = {
    .year = {'2','0','2','2','\0'},
    .month = {'J','A','N','\0'},
    .day = {'0','1','\0'},
    .hour0 = 1,
    .hour1 = 2,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'},
};
DateView start_date_view = {
    .year = {'2','0','2','2','\0'},
    .month = {'J','A','N','\0'},
    .day = {'0','1','\0'},
    .hour0 = 1,
    .hour1 = 2,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'},
};
DateView end_date_view = {
    .year = {'2','0','2','2','\0'},
    .month = {'J','A','N','\0'},
    .day = {'0','1','\0'},
    .hour0 = 1,
    .hour1 = 2,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'},
};
DateView new_date_view = {
    .year = {'2','0','2','2','\0'},
    .month = {'J','A','N','\0'},
    .day = {'0','1','\0'},
    .hour0 = 1,
    .hour1 = 2,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'},
};

void clock_init(void) {
    // set_timezone(TIMEZONE_DEFAULT);
    set_time(&current_date);
}

void set_timezone(const char* timezone) {
    setenv("TZ", timezone, 1);
    tzset();
}

void set_time(Date* date) {
    time_info = (struct tm) {
        .tm_sec = 0,
        .tm_min = date->minute,
        .tm_hour = date->hour,
        .tm_mday = date->day,
        .tm_mon = date->month - 1,
        .tm_year = date->year - 1900,
    };

    tv.tv_sec = mktime(&time_info);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}

void update_year_view(Date* date, DateView* date_view) {
    uint16_t temp_year = date->year;
    uint16_t year_digit = temp_year / 1000;
    date_view->year[0] = INT_TO_ASCII(year_digit);
    temp_year -= year_digit * 1000;

    year_digit = temp_year / 100;
    date_view->year[1] = INT_TO_ASCII(year_digit);
    temp_year -= year_digit * 100;

    year_digit = temp_year / 10;
    date_view->year[2] = INT_TO_ASCII(year_digit);
    date_view->year[3] = INT_TO_ASCII(temp_year - year_digit * 10);
    date_view->year[4] = '\0';
}

void update_month_view(Date* date, DateView* date_view) {
    switch (date->month) {
        case 1:
            date_view->month[0] = 'J';
            date_view->month[1] = 'A';
            date_view->month[2] = 'N';
            break;
        case 2:
            date_view->month[0] = 'F';
            date_view->month[1] = 'E';
            date_view->month[2] = 'B';
            break;
        case 3:
            date_view->month[0] = 'M';
            date_view->month[1] = 'A';
            date_view->month[2] = 'R';
            break;
        case 4:
            date_view->month[0] = 'A';
            date_view->month[1] = 'P';
            date_view->month[2] = 'R';
            break;
        case 5:
            date_view->month[0] = 'M';
            date_view->month[1] = 'A';
            date_view->month[2] = 'Y';
            break;
        case 6:
            date_view->month[0] = 'J';
            date_view->month[1] = 'U';
            date_view->month[2] = 'N';
            break;
        case 7:
            date_view->month[0] = 'J';
            date_view->month[1] = 'U';
            date_view->month[2] = 'L';
            break;
        case 8:
            date_view->month[0] = 'A';
            date_view->month[1] = 'U';
            date_view->month[2] = 'G';
            break;
        case 9:
            date_view->month[0] = 'S';
            date_view->month[1] = 'E';
            date_view->month[2] = 'P';
            break;
        case 10:
            date_view->month[0] = 'O';
            date_view->month[1] = 'C';
            date_view->month[2] = 'T';
            break;
        case 11:
            date_view->month[0] = 'N';
            date_view->month[1] = 'O';
            date_view->month[2] = 'V';
            break;
        case 12:
            date_view->month[0] = 'D';
            date_view->month[1] = 'E';
            date_view->month[2] = 'C';
            break;
    }
    date_view->month[3] = '\0';
}

void update_day_view(Date* date, DateView* date_view) {
    uint16_t day_digit = date->day / 10;
    date_view->day[0] = INT_TO_ASCII(day_digit);
    date_view->day[1] = INT_TO_ASCII(date->day - day_digit * 10);
    date_view->day[2] = '\0';
}

void update_hour_view(Date* date, DateView* date_view) {
    uint16_t temp_hour = date->hour;
    if (temp_hour < 12) {
        if (temp_hour == 0) {
            temp_hour = 12;
        }
        date_view->suffix[0] = 'A';
    }
    else {
        if (temp_hour != 12) {
            temp_hour -= 12;
        }
        date_view->suffix[0] = 'P';
    }
    date_view->hour0 = temp_hour / 10;
    date_view->hour1 = temp_hour - 10 * date_view->hour0;
    date_view->suffix[1] = 'M';
    date_view->suffix[2] = '\0';
}

void update_minute_view(Date* date, DateView* date_view) {
    uint16_t temp_minute = date->minute;
    date_view->minute0 = temp_minute / 10;
    date_view->minute1 = temp_minute - 10 * date_view->minute0;
}

void update_time(void) {
    // get the new raw time (time_t) and translate to time_info (struct tm)
    time(&raw_time);
    localtime_r(&raw_time, &time_info);

    current_date.year = time_info.tm_year + 1900;
    current_date.month = time_info.tm_mon + 1;
    current_date.day = time_info.tm_mday;
    current_date.hour = time_info.tm_hour;
    current_date.minute = time_info.tm_min;
}

void update_view(Date* date, DateView* date_view) {
    update_year_view(date, date_view);
    update_month_view(date, date_view);
    update_day_view(date, date_view);
    update_hour_view(date, date_view);
    update_minute_view(date, date_view);
}