#ifndef SMARTPOTS_MESSAGE_H
#define SMARTPOTS_MESSAGE_H
#include <cstddef>
#include "measurement.h"

bool parseMessage(void *data, size_t size, Message& message);

#endif //SMARTPOTS_MESSAGE_H
