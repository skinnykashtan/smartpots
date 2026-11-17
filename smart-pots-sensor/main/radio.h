#ifndef SMART_POTS_GATEWAY_RADIO_H
#define SMART_POTS_GATEWAY_RADIO_H
#include <cstdio>
#include "measurement.h"

void radio_init();
bool radio_send(int raw_data, uint8_t id);


#endif //SMART_POTS_GATEWAY_RADIO_H
