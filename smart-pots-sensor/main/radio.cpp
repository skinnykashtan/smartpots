#include "radio.h"
#include "esp_now.h"
#include "measurement.h"
#include "esp_wifi.h"
#include <cstring>
#include "espnow.h"

namespace {

    constexpr uint8_t gateway_broadcast_mac[6] = {
         0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF
    };

}

void radio_init() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE));


    espnow_config_t config = ESPNOW_INIT_CONFIG_DEFAULT();
    config.sec_enable = 1;

    ESP_ERROR_CHECK(espnow_init(&config));

    esp_now_peer_info_t peer_info{};

    memcpy(peer_info.peer_addr, gateway_broadcast_mac, sizeof(gateway_broadcast_mac));
    peer_info.channel = 11;
    peer_info.ifidx = WIFI_IF_STA;
    peer_info.encrypt = false;

    // ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));
}

bool radio_send(int raw_data, uint8_t id) {
    Message message{
        .plant_id = id,
        .data = raw_data
    };

    espnow_frame_head_t frame_config = ESPNOW_FRAME_CONFIG_DEFAULT();

    // return esp_now_send(gateway_broadcast_mac,
    //     reinterpret_cast<const uint8_t*>(&message),
    //     sizeof(Message)
    //     ) == ESP_OK;

    return espnow_send(ESPNOW_DATA_TYPE_DATA,
        gateway_broadcast_mac,
        &message,
        sizeof(Message),
        &frame_config,
        1000
        ) == ESP_OK;
}
