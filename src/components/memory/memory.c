#include "memory.h"

Schedule schedule;
uint32_t food_amount = 10;
uint32_t water_amount = 10;
uint8_t feed_select = -1;
uint8_t feed_toggle[5] = {0, 0, 0, 0, 0};

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
Date feed_date = {
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
DateView feed_date_view = {
    .year = {'2','0','2','2','\0'},
    .month = {'J','A','N','\0'},
    .day = {'0','1','\0'},
    .hour0 = 1,
    .hour1 = 2,
    .minute0 = 0,
    .minute1 = 0,
    .suffix = {'A','M','\0'},
};

void select_feeding_time(FeedingTime* feeding_time, DateView* feed_date_view) {
    feed_date.hour = feeding_time->hour;
    feed_date.minute = feeding_time->minute;
    food_amount = feeding_time->food_amount;
    water_amount = feeding_time->water_amount;
}

void memory_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    for (int i = 0; i < 5; i++) {
        schedule.feeding_times[i].hour = 0;
        schedule.feeding_times[i].minute = 0;
        schedule.feeding_times[i].food_amount = 10;
        schedule.feeding_times[i].water_amount = 10;
    }

    size_t length;
    uint32_t* data;
    data = mem_read(SCHEDULE_NAMESPACE, STORAGE_KEY, &length);
    if (length > 0) {
        memcpy(&schedule, data, length * sizeof(uint32_t));
    }
    if (data != NULL) { free(data); }
}

void mem_write(const char* namespace, const char* key, uint32_t* new_data, size_t length) {
    printf("\nCalling mem_write()\n");
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return;
    }
    printf("Done\n");

    // Read size of existing data from NVS
    printf("Reading size of existing data ... ");
    size_t required_size = 0;
    err = nvs_get_blob(handle, key, NULL, &required_size);
    switch (err) {
        case ESP_OK:
            printf("%d bytes\n", 4 * required_size);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value has not been initialized\n");
            break;
        default:
            printf("Error (%s) reading size\n", esp_err_to_name(err));
            nvs_close(handle);
            return;
    }

    // Read existing data from NVS
    printf("Reading existing data from NVS ... ");
    size_t required_length = required_size / sizeof(uint32_t);
    size_t new_size = length * sizeof(uint32_t);
    size_t total_size = required_size + new_size;
    uint32_t* data = malloc(total_size);
    if (required_size > 0) {
        err = nvs_get_blob(handle, key, data, &required_size);
        if (err != ESP_OK) {
            printf("Error (%s) reading data\n", esp_err_to_name(err));
            free(data);
            return;
        }
        printf("Done\n");
    }
    else {
        printf("Nothing saved yet\n");
    }

    // Append new data to existing data
    printf("Appending new data to existing data ... ");
    for (int i = 0; i < length; i++) {
        data[required_length + i] = new_data[i];
    }
    printf("%d bytes appended\n", new_size);

    // Write combined data to NVS
    printf("Writing data to NVS ... ");
    err = nvs_set_blob(handle, key, data, total_size);
    free(data);
    if (err != ESP_OK) {
        printf("Error (%s) writing\n", esp_err_to_name(err));
        nvs_close(handle);
        return;
    }
    printf("%d bytes written\n", total_size);

    // Storage not updated until nvs_commit is called
    printf("Committing data to NVS ... ");
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing\n", esp_err_to_name(err));
        nvs_close(handle);
        return;
    }
    printf("Done\n");

    printf("Closing NVS handle ... ");
    nvs_close(handle);
    printf("Done\n");
}

uint32_t* mem_read(const char* namespace, const char* key, size_t* length) {
    printf("\nCalling mem_read()\n");
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return NULL;
    }
    printf("Done\n");

    printf("Getting size of data to be read ... ");
    // get required_size
    size_t required_size = 0;
    err = nvs_get_blob(handle, key, NULL, &required_size);
    switch (err) {
        case ESP_OK:
            printf("%d bytes\n", required_size);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value has not been initialized\n");
            break;
        default:
            printf("Error (%s) reading\n", esp_err_to_name(err));
            nvs_close(handle);
            return NULL;
    }
    *length = required_size / sizeof(uint32_t);

    // Read data from NVS
    uint32_t* data = NULL;
    printf("Reading data from NVS ... ");
    if (required_size == 0) {
        printf("Nothing saved yet\n");
    }
    else {
        data = malloc(required_size);
        err = nvs_get_blob(handle, key, data, &required_size);
        if (err != ESP_OK) {
            free(data);
            nvs_close(handle);
            return NULL;
        }
        printf("%d bytes read\n", required_size);
    }

    printf("Closing NVS handle ... ");
    nvs_close(handle);
    printf("Done\n");
    return data;
}

void mem_erase(const char* namespace) {
    printf("\nCalling mem_erase()\n");
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return;
    }
    printf("Done\n");

    printf("Erasing data associated with NVS handle ... ");
    err = nvs_erase_all(handle);
    if (err != ESP_OK) {
        printf("Error (%s) erasing data\n", esp_err_to_name(err));
        return;
    }
    printf("Done\n");
}