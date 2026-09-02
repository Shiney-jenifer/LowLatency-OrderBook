#pragma once
#include <cstdint>

enum class Side : uint8_t {
    Buy  = 0,
    Sell = 1
};

struct Order {
    uint64_t orderId;
    Side     side;
    uint32_t quantity;
    double   price;
    uint64_t seqNum;   // insertion sequence for age-based priority
};
