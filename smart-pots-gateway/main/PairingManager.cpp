#include "PairingManager.h"

#include "esp_mac.h"
#include "esp_random.h"
#include <cstring>

namespace {
    constexpr const char* TAG = "PairingManager";
}

PairingManager::~PairingManager() {
    ESP_FREE(addrs_list);
    ESP_FREE(paired_addrs_list);
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

void PairingManager::printMAC() {
    for (int i=0; i<num; ++i) {
        ESP_LOGI(TAG, "Sensor %zu MAC: " MACSTR,
            i, MAC2STR(addrs_list[i])
        );
    }
}

esp_err_t PairingManager::start() {
    if (addrs_list == nullptr || num == 0) {
        return ESP_ERR_INVALID_STATE;
    }

    espnow_sec_result_t res{};
    uint8_t key_info[APP_KEY_LEN];
    esp_fill_random(key_info, APP_KEY_LEN);

    esp_err_t result = espnow_sec_initiator_start(key_info, pop, addrs_list, num, &res);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "error during initiator_start() err code=%s", esp_err_to_name(result));
        espnow_sec_initiator_result_free(&res);
        return result;
    }

    if (res.successed_num == 0) {
        ESP_LOGW(TAG, "No successful handshakes. Check PoP and responder status.");
        espnow_sec_initiator_result_free(&res);
        return ESP_FAIL;
    }

    paired_addrs_list = static_cast<espnow_addr_t*>(ESP_MALLOC(res.successed_num * ESPNOW_ADDR_LEN));

    if (paired_addrs_list == nullptr) {
        ESP_LOGE(TAG, "error during allocating memory");
        espnow_sec_initiator_result_free(&res);
        return ESP_ERR_ESPNOW_NO_MEM;
    }

    for (size_t i=0; i<res.successed_num; ++i) {
        ESP_LOGI(TAG, "Succeeded handshakes MACs: " MACSTR, MAC2STR(res.successed_addr[i]));
        memcpy(paired_addrs_list[i], res.successed_addr[i], ESPNOW_ADDR_LEN);
    }

    espnow_sec_initiator_result_free(&res);

    esp_err_t err = espnow_set_key(key_info);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error during saving APP_KEY to NVS");
        return err;
    }

    return ESP_OK;
}
