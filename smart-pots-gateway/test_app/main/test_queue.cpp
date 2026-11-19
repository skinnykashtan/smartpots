#include "unity.h"
#include "measurement.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

TEST_CASE("queue - send and receive message", "[queue]") {
    QueueHandle_t queue = xQueueCreate(
        5,
        sizeof(Message)
    );
    TEST_ASSERT(queue != nullptr);

    Message input {
        .plant_id = 1,
        .data = 1337
    };

    BaseType_t send = xQueueSend(
        queue,
        &input,
        0
    );
    TEST_ASSERT(send == pdPASS);

    Message output{};
    BaseType_t recv = xQueueReceive(
        queue,
        &output,
        0
    );
    TEST_ASSERT(recv == pdPASS);

    TEST_ASSERT_EQUAL_UINT8(1, output.plant_id);
    TEST_ASSERT_EQUAL_UINT32(1337, output.data);

    vQueueDelete(queue);
}