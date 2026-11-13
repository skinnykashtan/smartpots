#ifndef SMART_POTS_GATEWAY_RADIO_H
#define SMART_POTS_GATEWAY_RADIO_H
#include <cstdio>

struct Payload {
    uint16_t raw;
};

void radio_init();
int radio_send(Payload payload);


#endif //SMART_POTS_GATEWAY_RADIO_H
