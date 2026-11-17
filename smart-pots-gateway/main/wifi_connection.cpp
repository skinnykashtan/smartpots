#include "wifi_connection.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace wifi {

    namespace {
        constexpr char* ssid = CONFIG_ESP_WIFI_SSID;
        constexpr char* password = CONFIG_ESP_WIFI_PASSWORD;
        constexpr int max_retries = CONFIG_ESP_MAXIMUM_RETRY;

        constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;
        constexpr EventBits_t WIFI_FAIL_BIT = BIT1;

        EventGroupHandle_t wifi_event_group = nullptr;
        int retry_count = 0;

        constexpr char* TAG = "WIFI";

        void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
                esp_wifi_connect();
            } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
                if (retry_count < max_retries) {
                    esp_wifi_connect();
                    retry_count++;
                    ESP_LOGI(TAG, "retry to connect to the AP");
                } else {
                    xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                }
                ESP_LOGI(TAG, "connect to the AP fail");
            } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                auto* event = static_cast<ip_event_got_ip_t*>(event_data);
                ESP_LOGI(TAG, "got ip:" IPSTR " | Mask:" IPSTR, IP2STR(&event->ip_info.ip), IP2STR(&event->ip_info.netmask));
                retry_count = 0;
                xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            }
        }
    }

    bool wifi_init_sta() {
        wifi_event_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_netif_init());

        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            nullptr,
            &instance_any_id
            ));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            nullptr,
            &instance_got_ip
            ));

        wifi_config_t wifi_config = {
            .sta = {
                .ssid = CONFIG_ESP_WIFI_SSID,
                .password = CONFIG_ESP_WIFI_PASSWORD,
            },
        };
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "wifi_init_sta finished");

        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
            );

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "connected to AP SSID: %s", CONFIG_ESP_WIFI_SSID);

            uint8_t channel = 0;
            wifi_second_chan_t second_channel;

            ESP_ERROR_CHECK(
                esp_wifi_get_channel(&channel, &second_channel)
            );

            ESP_LOGI("WIFI", "Gateway channel: %u", channel);

            return true;
        }
        if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "failed to connect to AP SSID: %s", CONFIG_ESP_WIFI_SSID);
            return false;
        }

        ESP_LOGE(TAG, "Unexpected Event");
        return false;
    }

}
