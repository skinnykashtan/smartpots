#include "radio.h"

void radio_init() {

}

int radio_send(int raw) {
    Payload payload{};
    payload.raw = static_cast<uint16_t>(raw);

    return payload.raw;
}
