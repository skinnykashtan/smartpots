#ifndef SMARTPOTS_MQTT_CLIENT_H
#define SMARTPOTS_MQTT_CLIENT_H
#include "measurement.h"

namespace mqtt {

    bool mqtt_init();
    void mqtt_publisher_init();
    void measurement_queue_init();
    bool enqueue_measurement(const Message& message);

}

#endif //SMARTPOTS_MQTT_CLIENT_H
