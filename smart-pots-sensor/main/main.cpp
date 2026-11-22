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
    ESP_ERROR_CHECK(nvs_flash_init());

    soil_sensor_init();
    radio_init();
    espnow_sec_responder_start(pop);

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
