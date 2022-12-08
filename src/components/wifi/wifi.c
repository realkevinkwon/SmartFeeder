#include <wifi.h>

// typedef struct _Time {
//     uint8_t hour0;
//     uint8_t hour1;
//     uint8_t minute0;
//     uint8_t minute1;
//     char suffix[3];
// } Time;
time_t raw_time;
struct tm* time_info;
Time current_time;
// Time current_time = {
//     .hour0 = 0,
//     .hour1 = 0,
//     .minute0 = 0,
//     .minute1 = 0,
//     .suffix = {'A','M','\0'}
// };

void wifi_init(void) {
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void update_time(void) {
    // get the new raw time
    time(&raw_time);

    // translate raw_time (time_t) to time_info (struct tm*)
    time_info = localtime(&raw_time);
    uint8_t temp_hour = time_info->tm_hour;
    uint8_t temp_minute = time_info->tm_min;

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