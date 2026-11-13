#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "radio.h"
#include "soil_sensor.h"

extern "C" void app_main(void) {

    soil_sensor_init();
    radio_init();


    while (true) {
        int raw = soil_sensor_read();
        ESP_LOGI("sensor", "raw: %d", raw);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
