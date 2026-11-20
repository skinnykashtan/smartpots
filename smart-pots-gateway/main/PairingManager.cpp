#include "PairingManager.h"

namespace {
    constexpr const char* TAG = "PairingManager";
}

PairingManager::~PairingManager() {
    ESP_FREE(addrs_list);
}

esp_err_t PairingManager::scan() {
    size_t new_num = 0;
    espnow_sec_responder_t* info_list = nullptr;

    esp_err_t res = espnow_sec_initiator_scan(
            &info_list,
            &new_num,
            pdMS_TO_TICKS(3000)
        );

    if (res != ESP_OK) {
        return res;
    }

    ESP_LOGI(TAG, "Found devices num=%zu", new_num);

    if (new_num == 0) {
        espnow_sec_initiator_scan_result_free();
        return ESP_ERR_ESPNOW_NOT_FOUND;
    }

    espnow_addr_t* new_addrs = static_cast<espnow_addr_t*>(ESP_MALLOC(new_num * ESPNOW_ADDR_LEN));

    if (new_addrs == nullptr) {
        espnow_sec_initiator_scan_result_free();
        return ESP_ERR_NO_MEM;
    }

    for (size_t i=0; i<new_num; ++i) {
        memcpy(new_addrs[i], info_list[i].mac, ESPNOW_ADDR_LEN);
    }

    ESP_FREE(addrs_list);
    addrs_list = new_addrs;
    num = new_num;

    espnow_sec_initiator_scan_result_free();

    return ESP_OK;
}
