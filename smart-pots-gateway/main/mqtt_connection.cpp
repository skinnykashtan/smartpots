#include "mqtt_connection.h"

#include "esp_log.h"
#include "mqtt_client.h"

namespace mqtt {

    constexpr const char* TAG = "MQTT";

    namespace {

        void mqtt_publisher_task(void* arg);

        QueueHandle_t measurement_queue = nullptr;

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

    bool mqtt_init() {
        mqtt_event_group = xEventGroupCreate();

        client = esp_mqtt_client_init(&mqtt_cfg);
        if (client == nullptr) {
            ESP_LOGI(TAG, "Client Initialization failed");
            return false;
        }

        ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, client));
        ESP_ERROR_CHECK(esp_mqtt_client_start(client));

        ESP_LOGI(TAG, "client started");
        return true;
    }

    void measurement_queue_init() {
        measurement_queue = xQueueCreate(
            5,
            sizeof(Message)
        );

        configASSERT(measurement_queue != nullptr);
    }

    void mqtt_publisher_init() {
        BaseType_t ret = xTaskCreate(
            mqtt_publisher_task,
            "mqtt_publisher",
            4096,
            measurement_queue,
            5,
            nullptr
        );

        configASSERT(ret == pdPASS);
    }

    bool enqueue_measurement(const Message& message) {
        return xQueueSend(
            measurement_queue,
            &message,
            0
            ) == pdPASS;
    }

    namespace {

        void mqtt_publisher_task(void* arg) {
            auto queue = static_cast<QueueHandle_t>(arg);
            Message message{};
            char payload[16];

            while (true) {
                BaseType_t ret = xQueueReceive(
                    queue,
                    &message,
                    portMAX_DELAY
                );

                configASSERT(ret == pdPASS);

                xEventGroupWaitBits(
                mqtt_event_group,
                MQTT_CONNECTED_BIT,
                pdFALSE,
                pdTRUE,
                portMAX_DELAY
                );

                snprintf(
                    payload,
                    sizeof(payload),
                    "%ld",
                    static_cast<long>(message.data)
                );

                esp_mqtt_client_publish(client, "smartpots/saloon/plant1", payload, 0, 1, true);
            }
        }

    }

}
