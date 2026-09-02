#pragma once
#include "Order.h"
#include <cstdint>
#include <vector>

/**
 * OrderPool  —  fixed-size slab allocator for Order nodes
 * Capacity: MAX_ORDERS concurrent resting orders (tune as needed).
 */

static constexpr uint32_t NULL_IDX   = UINT32_MAX;
static constexpr uint32_t MAX_ORDERS = 1 << 20;  // 1M slots (~64 MB)

struct OrderNode {
    Order    order;
    uint32_t prev  = NULL_IDX;
    uint32_t next  = NULL_IDX;
    bool     inUse = false;
};

class OrderPool {
public:
    OrderPool() : slots_(MAX_ORDERS), freeHead_(0) {
        // Build the free list: slot 0 → 1 → 2 → ... → MAX_ORDERS-1 → NULL
        for (uint32_t i = 0; i < MAX_ORDERS - 1; ++i)
            slots_[i].next = i + 1;
        slots_[MAX_ORDERS - 1].next = NULL_IDX;
    }

    // Returns the slot index, or NULL_IDX if the pool is exhausted.
    // The caller MUST check for NULL_IDX and reject the order gracefully.
    // The branch below is virtually never taken, so the CPU branch predictor
    // makes it effectively free on the hot path — zero latency impact.
    uint32_t alloc(const Order& o) {
        if (freeHead_ == NULL_IDX)        // pool full — handled, never crashes
            return NULL_IDX;
        uint32_t idx = freeHead_;
        freeHead_ = slots_[idx].next;
        slots_[idx].order  = o;
        slots_[idx].prev   = NULL_IDX;
        slots_[idx].next   = NULL_IDX;
        slots_[idx].inUse  = true;
        return idx;
    }

    void free(uint32_t idx) {
        if (idx >= MAX_ORDERS || !slots_[idx].inUse)
            return;                       // ignore invalid / double free safely
        slots_[idx].inUse = false;
        slots_[idx].next  = freeHead_;
        freeHead_ = idx;
    }

    OrderNode&       operator[](uint32_t idx)       { return slots_[idx]; }
    const OrderNode& operator[](uint32_t idx) const { return slots_[idx]; }

private:
    std::vector<OrderNode> slots_;   // heap-allocated contiguous slab
    uint32_t freeHead_ = 0;
};
