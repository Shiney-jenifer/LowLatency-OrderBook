#pragma once
#include "Order.h"
#include <vector>
#include <ostream>

// One price level: a price + the orders waiting at that price (FIFO)
struct Level {
    int64_t price;
    std::vector<Order> orders;   // index 0 = oldest (matched first)
};

class OrderBook {
    private:
        std::vector<Level> bids_;   // buy side  — sorted HIGH to LOW (best bid at [0])
        std::vector<Level> asks_;   // sell side — sorted LOW to HIGH (best ask at [0])
        std::ostream& out_;
        void rest(const Order& order);
        void match(Order& incoming, std::vector<Level>& oppSide);
        bool cancelFrom(std::vector<Level>& lvlside, uint64_t orderId);
    public:
        explicit OrderBook(std::ostream& out) : out_(out) {
            bids_.reserve(256);
            asks_.reserve(256);
        }
        void add(const Order& order);
        void cancel(uint64_t orderId);
};

