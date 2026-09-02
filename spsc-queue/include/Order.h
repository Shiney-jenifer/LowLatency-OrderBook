#pragma once
#include <cstdint>

enum class Side:uint8_t {
    Buy  = 0,
    Sell = 1
};

enum class Message:uint8_t {
    Add = 1,
    Cancel = 0
};

struct Order {
    uint64_t orderId;
    int64_t   price;
    uint32_t quantity;
    Side     side;
    Message  msg;
};
