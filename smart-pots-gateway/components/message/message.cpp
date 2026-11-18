#include <cstring>
#include "measurement.h"

bool parseMessage(void *data, size_t size, Message& message) {
    if (data == nullptr || size != sizeof(Message)) {
        return false;
    }

    memcpy(&message, data, sizeof(message));
    return true;
}