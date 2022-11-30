#include "memory.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"

#define STORAGE_NAMESPACE "storage"

void mem_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void mem_write(const char* namespace, uint32_t* data) {
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return;
    }
    printf("Done\n");

    printf("Writing to NVS ... ");

    printf("Closing NVS handle ... ");
    nvs_close(handle);
    printf("Done\n");
}

void mem_read(const char* namespace, uint32_t* data) {
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return;
    }
    printf("Done\n");

    printf("Getting size of data to be read ... ");
    size_t required_size = 0;   // size of data to be read
    // get required_size
    err = nvs_get_blob(handle, namespace, NULL, &required_size);
    switch (err) {
        case ESP_OK:
            printf("Done\n");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet\n");
            nvs_close(handle);
            return;
        default:
            printf("Error (%s) reading\n", esp_err_to_name(err));
            nvs_close(handle);
            return;
    }
    printf("Reading values from NVS ... ");
    if (required_size == 0) {
        printf("Nothing saved yet\n");
    }
    else {
        data = malloc(required_size);
        err = nvs_get_blob(handle, namespace, data, &required_size);
        if (err != ESP_OK) {
            free(data);
            nvs_close(handle);
            return;
        }
        printf("Done\n");
    }

    printf("Closing NVS handle ... ");
    nvs_close(handle);
    printf("Done\n");
}