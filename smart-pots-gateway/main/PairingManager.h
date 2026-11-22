#ifndef SMARTPOTS_PAIRINGMANAGER_H
#define SMARTPOTS_PAIRINGMANAGER_H
#include "espnow_security_handshake.h"

class PairingManager {
private:
    espnow_addr_t* addrs_list = nullptr;
    size_t num = 0;
    static constexpr const char* pop = "cos";
    espnow_addr_t* paired_addrs_list = nullptr;
public:
    PairingManager() = default;

    ~PairingManager();

    PairingManager(PairingManager&&) = delete;
    PairingManager& operator=(PairingManager&&) = delete;
    PairingManager(const PairingManager&) = delete;
    PairingManager& operator=(const PairingManager&) = delete;

    esp_err_t scan();
    esp_err_t start();
    void printMAC();
};

#endif //SMARTPOTS_PAIRINGMANAGER_H
