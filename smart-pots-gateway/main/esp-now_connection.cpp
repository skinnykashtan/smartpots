#include "esp-now_connection.h"
#include "esp_now.h"
#include "measurement.h"
#include <cstring>
#include "mqtt_connection.h"

namespace espnow {

    constexpr const char* TAG = "ESP-NOW";

    namespace {

        void espnow_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {
            const uint8_t* mac_addr = esp_now_info->src_addr;
            const uint8_t* des_addr = esp_now_info->des_addr;
            Message message;

            if (data != nullptr && data_len == sizeof(Message)) {
                memcpy(&message, data, sizeof(Message));

                mqtt::enqueue_measurement(message);
            }
        }

    }

    void espnow_init() {
        ESP_ERROR_CHECK(esp_now_init());

        ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    }

}
