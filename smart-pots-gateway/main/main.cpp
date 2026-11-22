#include <cstdio>
#include <nvs_flash.h>
#include "esp_log.h"
#include "mqtt_connection.h"
#include "wifi_connection.h"
#include "measurement.h"
#include "esp-now_connection.h"
#include "PairingManager.h"

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

    // ESP_ERROR_CHECK(espnow_erase_key());
    ESP_ERROR_CHECK(espnow_storage_init());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi::wifi_init_sta();

    mqtt::measurement_queue_init();
    mqtt::mqtt_init();
    mqtt::mqtt_publisher_init();

    espnow::initEspNow();

    uint8_t key_info[APP_KEY_LEN];
    if (espnow_get_key(key_info) != ESP_OK) {
        ESP_LOGI(TAG, "No APP KEY in NVS, waiting for pairing...");
        PairingManager pairingManager;

        ESP_LOGI(TAG, "Before scan");

        esp_err_t res = pairingManager.scan();

        ESP_LOGI(TAG, "After scan: %s", esp_err_to_name(res));

        if (res == ESP_OK) {
            pairingManager.printMAC();
            pairingManager.start();
        } else {
            ESP_LOGW(TAG, "no devices to start pairing");
        }
    } else {
        ESP_ERROR_CHECK(espnow_set_key(key_info));
        ESP_LOGI(TAG, "APP KEY found in NVS.");
    }

}
