#include "mqtt_connection.h"

#include "esp_log.h"
#include "mqtt_client.h"

namespace mqtt {

    constexpr const char* TAG = "MQTT";

    namespace {

        constexpr EventBits_t MQTT_CONNECTED_BIT = BIT0;

        esp_mqtt_client_config_t mqtt_cfg = {
            .broker = {
                .address = {
                    .uri = "mqtt://192.168.100.5:1883"
                }
            }
        };
        esp_mqtt_client_handle_t client = nullptr;
        EventGroupHandle_t mqtt_event_group = nullptr;

        int msg_id = -1;

        void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
            ESP_LOGI(TAG, "event handler call");
            auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
            auto handler_client = event->client;

            switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
                case MQTT_EVENT_CONNECTED:
                    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
                    xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
                    msg_id = esp_mqtt_client_publish(
                        client,
                        "smartpots/bathroom/plant1",
                        "42",
                        0,
                        1,
                        1
                        );
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                    break;
                case MQTT_EVENT_DISCONNECTED:
                    xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
                    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
                    break;
                case MQTT_EVENT_PUBLISHED:
                    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);

                    if (msg_id == event->msg_id) {
                        ESP_LOGI(TAG, "Correct message");
                    }
                    break;
                default:
                    ESP_LOGI(TAG, "not handled type: %ld", static_cast<long>(event_id));
                    break;
            }
        }

    }

    void mqtt_init() {
        mqtt_event_group = xEventGroupCreate();

        client = esp_mqtt_client_init(&mqtt_cfg);
        if (client == nullptr) {
            ESP_LOGI(TAG, "Client Initialization failed");
            return;
        }

        ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, client));
        ESP_ERROR_CHECK(esp_mqtt_client_start(client));

        xEventGroupWaitBits(
            mqtt_event_group,
            MQTT_CONNECTED_BIT,
            pdFALSE,
            pdTRUE,
            portMAX_DELAY
            );

       ESP_LOGI(TAG, "Connected to the broker");
    }

}
