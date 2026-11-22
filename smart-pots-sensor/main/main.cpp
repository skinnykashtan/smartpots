#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "radio.h"
#include "soil_sensor.h"
#include <nvs_flash.h>
#include "espnow_security_handshake.h"

constexpr const char* TAG = "SENSOR";
constexpr const char* pop = "cos";

extern "C" void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // ESP_ERROR_CHECK(espnow_erase_key());
    ESP_ERROR_CHECK(espnow_storage_init());

    soil_sensor_init();
    radio_init();

    uint8_t key_info[APP_KEY_LEN];
    if (espnow_get_key(key_info) != ESP_OK) {
        ESP_LOGI(TAG, "No APP KEY in NVS, waiting for pairing...");
        espnow_sec_responder_start(pop);
    } else {
        ESP_ERROR_CHECK(espnow_set_key(key_info));
        ESP_LOGI(TAG, "APP KEY found in NVS.");
    }

    // while (true) {
    //     int raw = soil_sensor_read();
    //     ESP_LOGI("sensor", "raw: %d", raw);
    //
    //     if (!radio_send(raw, 1)) {
    //         ESP_LOGE(TAG, "ESP-NOW send failed");
    //     }
    //
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}
