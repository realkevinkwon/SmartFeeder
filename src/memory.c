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

void mem_write(const char* namespace, uint32_t* new_data, size_t length) {
    printf("\nCalling mem_write()\n");
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return;
    }
    printf("Done\n");

    // Read size of existing data from NVS
    printf("Reading size of existing data ... ");
    size_t required_size = 0;
    err = nvs_get_blob(handle, namespace, NULL, &required_size);
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
        err = nvs_get_blob(handle, namespace, data, &required_size);
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
    err = nvs_set_blob(handle, namespace, data, total_size);
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

uint32_t* mem_read(const char* namespace, size_t* length) {
    printf("\nCalling mem_read()\n");
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle\n", esp_err_to_name(err));
        return NULL;
    }
    printf("Done\n");

    printf("Getting size of data to be read ... ");
    // get required_size
    size_t required_size = 0;
    err = nvs_get_blob(handle, namespace, NULL, &required_size);
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
        err = nvs_get_blob(handle, namespace, data, &required_size);
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

void mem_erase(void) {
    printf("\nCalling mem_erase()\n");
    printf("Opening Non-Volatile Storage (NVS) handle ... ");
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
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