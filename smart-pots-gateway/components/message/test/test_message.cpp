#include "unity.h"
#include "message.h"

TEST_CASE("parseMessage - nullptr", "[message]") {
    Message message{
        .plant_id = 1,
        .data = 3333
    };

    TEST_ASSERT_FALSE(parseMessage(nullptr, sizeof(message), message));

    TEST_ASSERT_EQUAL_UINT8(1, message.plant_id);
    TEST_ASSERT_EQUAL_UINT32(3333, message.data);
}

TEST_CASE("parseMessage - correct message", "[message]") {
    Message input{
        .plant_id = 1,
        .data = 3333
    };

    Message output{};

    TEST_ASSERT_TRUE(parseMessage(&input, sizeof(input), output));

    TEST_ASSERT_EQUAL_UINT8(1, output.plant_id);
    TEST_ASSERT_EQUAL_UINT32(3333, output.data);
}

TEST_CASE("parseMessage - wrong sizeof", "[message]") {
    Message input{
        .plant_id = 1,
        .data = 3333
    };

    Message output{
        .plant_id = 3,
        .data = 8888
    };

    TEST_ASSERT_FALSE(parseMessage(&input, sizeof(input) - 1, output));

    TEST_ASSERT_EQUAL_UINT8(3, output.plant_id);
    TEST_ASSERT_EQUAL_UINT32(8888, output.data);
}
