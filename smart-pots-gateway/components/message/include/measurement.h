#ifndef SMARTPOTS_MEASUREMENT_H
#define SMARTPOTS_MEASUREMENT_H
#include <cstdint>

struct Message {
    uint8_t plant_id;
    uint32_t data;
};

#endif //SMARTPOTS_MEASUREMENT_H
