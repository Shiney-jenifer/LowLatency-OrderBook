#pragma once
#include <cstdint>

enum class Side {
    Buy  = 0,
    Sell = 1
};

struct Order {
    uint64_t orderId;
    int64_t   price;
    Side     side;
    uint32_t quantity;
};
