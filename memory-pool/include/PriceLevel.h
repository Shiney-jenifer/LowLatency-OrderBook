#pragma once
#include "OrderPool.h"
#include <cstdint>

struct PriceLevel {
    double   price    = 0.0;
    uint32_t head     = NULL_IDX;   // oldest (front of queue)
    uint32_t tail     = NULL_IDX;   // newest (back of queue)
    uint32_t count    = 0;          // number of orders at this level

    bool empty() const { return head == NULL_IDX; }

    void push_back(uint32_t idx, OrderPool& pool) {
        auto& node = pool[idx];
        node.prev = tail;
        node.next = NULL_IDX;
        if (tail != NULL_IDX)
            pool[tail].next = idx;
        else
            head = idx;
        tail = idx;
        ++count;
    }

    // Remove a node by index (O(1) because we have prev/next).
    void remove(uint32_t idx, OrderPool& pool) {
        auto& node = pool[idx];
        if (node.prev != NULL_IDX) pool[node.prev].next = node.next;
        else                        head = node.next;
        if (node.next != NULL_IDX) pool[node.next].prev = node.prev;
        else                        tail = node.prev;
        --count;
    }
};
