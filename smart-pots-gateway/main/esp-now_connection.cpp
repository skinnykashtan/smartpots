#include "esp-now_connection.h"
#include "espnow.h"
#include "measurement.h"
#include <cstring>

#include "esp_log.h"
#include "mqtt_connection.h"
#include "message.h"

namespace espnow {

    constexpr const char* TAG = "ESP-NOW";

    namespace {

        esp_err_t espnow_recv_cb(uint8_t *src_addr, void *data, size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl) {
            Message message{};

            if (!parseMessage(data, size, message)) {
                return ESP_ERR_INVALID_ARG;
            }

            ESP_LOGI(TAG, "received id=%u, message=%ld", message.plant_id, message.data);

            mqtt::enqueue_measurement(message);

            return ESP_OK;
        }

    }

    void initEspNow() {
        espnow_config_t config = ESPNOW_INIT_CONFIG_DEFAULT();
        config.sec_enable = 1;

        ESP_ERROR_CHECK(espnow_init(&config));

        ESP_ERROR_CHECK(espnow_set_config_for_data_type(
            ESPNOW_DATA_TYPE_DATA,
            true,
            espnow_recv_cb));
    }

}
