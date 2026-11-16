#include <cstdio>
#include <nvs_flash.h>
#include "esp_log.h"
#include "mqtt_connection.h"
#include "wifi_connection.h"
#include "measurement.h"
#include "esp-now_connection.h"

Message message1 {
    .plant_id = 1,
    .data = "33"
};

Message message2 {
    .plant_id = 2,
    .data = "22"
};

Message message3 {
    .plant_id = 3,
    .data = "11"
};


namespace {
    constexpr const char* TAG = "MAIN";
}

extern "C" void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi::wifi_init_sta();

    mqtt::measurement_queue_init();
    mqtt::mqtt_init();
    mqtt::mqtt_publisher_init();

    espnow::espnow_init();
}
