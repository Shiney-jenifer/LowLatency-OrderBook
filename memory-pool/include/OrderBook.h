#pragma once
#include "Order.h"
#include "Messages.h"
#include "OrderPool.h"
#include "PriceLevel.h"
#include "Event.h"
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <algorithm>


template<typename THandler>
class OrderBook {
public:
    explicit OrderBook(THandler& handler) : handler_(handler) {
        buyLevels_.reserve(256);
        sellLevels_.reserve(256);
        index_.reserve(1 << 16);
    }

    void add   (const AddOrderRequest&    req, uint64_t seqNum);
    void cancel(const CancelOrderRequest& req);

    // Introspection for tests
    size_t       buyDepth()  const;
    size_t       sellDepth() const;
    const Order* find(uint64_t orderId) const;

private:
    THandler& handler_;

    std::vector<PriceLevel> buyLevels_;
    std::vector<PriceLevel> sellLevels_;
    OrderPool               pool_;
    std::unordered_map<uint64_t, uint32_t> index_;

    PriceLevel& getOrCreateBuyLevel (double price);
    PriceLevel& getOrCreateSellLevel(double price);
    void insertBuy (const Order& o);
    void insertSell(const Order& o);
    void matchBuyAggressor (Order aggressor);
    void matchSellAggressor(Order aggressor);
    void emitFill(uint64_t aggressorId, uint32_t tradeQty, double price,
                  uint32_t aggressorRemaining,
                  uint64_t restingId, uint32_t restingRemaining);
};

// ── Implementation (must be in header for templates) ─────────────────────────

template<typename THandler>
PriceLevel& OrderBook<THandler>::getOrCreateBuyLevel(double price) {
    auto it = std::lower_bound(buyLevels_.begin(), buyLevels_.end(), price,
        [](const PriceLevel& lvl, double p) { return lvl.price > p; });
    if (it != buyLevels_.end() && it->price == price) return *it;
    PriceLevel lvl; lvl.price = price;
    return *buyLevels_.insert(it, lvl);
}

template<typename THandler>
PriceLevel& OrderBook<THandler>::getOrCreateSellLevel(double price) {
    auto it = std::lower_bound(sellLevels_.begin(), sellLevels_.end(), price,
        [](const PriceLevel& lvl, double p) { return lvl.price < p; });
    if (it != sellLevels_.end() && it->price == price) return *it;
    PriceLevel lvl; lvl.price = price;
    return *sellLevels_.insert(it, lvl);
}

template<typename THandler>
void OrderBook<THandler>::insertBuy(const Order& o) {
    uint32_t slot = pool_.alloc(o);
    if (slot == NULL_IDX) {                                  // pool full
        handler_.onOrderRejected(OrderRejected(o.orderId, "order pool exhausted"));
        return;
    }
    getOrCreateBuyLevel(o.price).push_back(slot, pool_);
    index_[o.orderId] = slot;
}

template<typename THandler>
void OrderBook<THandler>::insertSell(const Order& o) {
    uint32_t slot = pool_.alloc(o);
    if (slot == NULL_IDX) {                                  // pool full
        handler_.onOrderRejected(OrderRejected(o.orderId, "order pool exhausted"));
        return;
    }
    getOrCreateSellLevel(o.price).push_back(slot, pool_);
    index_[o.orderId] = slot;
}

template<typename THandler>
void OrderBook<THandler>::emitFill(uint64_t aggressorId, uint32_t tradeQty,
                                   double price, uint32_t aggressorRemaining,
                                   uint64_t restingId, uint32_t restingRemaining) {
    // All three calls below are INLINED by the compiler — no virtual dispatch
    handler_.onTradeExecuted(TradeExecuted(tradeQty, price));

    if (aggressorRemaining == 0)
        handler_.onOrderFullyFilled(OrderFullyFilled(aggressorId));
    else
        handler_.onOrderPartiallyFilled(OrderPartiallyFilled(aggressorId, aggressorRemaining));

    if (restingRemaining == 0)
        handler_.onOrderFullyFilled(OrderFullyFilled(restingId));
    else
        handler_.onOrderPartiallyFilled(OrderPartiallyFilled(restingId, restingRemaining));
}

template<typename THandler>
void OrderBook<THandler>::matchBuyAggressor(Order aggressor) {
    while (aggressor.quantity > 0 && !sellLevels_.empty()) {
        PriceLevel& level = sellLevels_.front();
        if (level.price > aggressor.price) break;

        uint32_t slotIdx   = level.head;
        Order&   resting   = pool_[slotIdx].order;
        uint32_t tradeQty  = std::min(aggressor.quantity, resting.quantity);

        aggressor.quantity -= tradeQty;
        resting.quantity   -= tradeQty;

        uint64_t rid        = resting.orderId;
        uint32_t restingRem = resting.quantity;
        double   tradePrice = resting.price;

        if (resting.quantity == 0) {
            level.remove(slotIdx, pool_);
            index_.erase(rid);
            pool_.free(slotIdx);
            if (level.empty()) sellLevels_.erase(sellLevels_.begin());
        }

        emitFill(aggressor.orderId, tradeQty, tradePrice,
                 aggressor.quantity, rid, restingRem);
    }
    if (aggressor.quantity > 0) insertBuy(aggressor);
}

template<typename THandler>
void OrderBook<THandler>::matchSellAggressor(Order aggressor) {
    while (aggressor.quantity > 0 && !buyLevels_.empty()) {
        PriceLevel& level = buyLevels_.front();
        if (level.price < aggressor.price) break;

        uint32_t slotIdx   = level.head;
        Order&   resting   = pool_[slotIdx].order;
        uint32_t tradeQty  = std::min(aggressor.quantity, resting.quantity);

        aggressor.quantity -= tradeQty;
        resting.quantity   -= tradeQty;

        uint64_t rid        = resting.orderId;
        uint32_t restingRem = resting.quantity;
        double   tradePrice = resting.price;

        if (resting.quantity == 0) {
            level.remove(slotIdx, pool_);
            index_.erase(rid);
            pool_.free(slotIdx);
            if (level.empty()) buyLevels_.erase(buyLevels_.begin());
        }

        emitFill(aggressor.orderId, tradeQty, tradePrice,
                 aggressor.quantity, rid, restingRem);
    }
    if (aggressor.quantity > 0) insertSell(aggressor);
}

template<typename THandler>
void OrderBook<THandler>::add(const AddOrderRequest& req, uint64_t seqNum) {
    Order o{req.orderId, req.side, req.quantity, req.price, seqNum};
    handler_.onOrderAccepted(OrderAccepted(req.orderId, req.side, req.quantity, req.price));
    if (req.side == Side::Buy) matchBuyAggressor(o);
    else                       matchSellAggressor(o);
}

template<typename THandler>
void OrderBook<THandler>::cancel(const CancelOrderRequest& req) {
    auto it = index_.find(req.orderId);
    if (it == index_.end()) {
        handler_.onOrderCancelRejected(OrderCancelRejected(req.orderId, "unknown orderId"));
        return;
    }

    uint32_t slotIdx = it->second;
    double   price   = pool_[slotIdx].order.price;
    Side     side    = pool_[slotIdx].order.side;
    auto&    levels  = (side == Side::Buy) ? buyLevels_ : sellLevels_;

    auto lvlIt = std::find_if(levels.begin(), levels.end(),
        [price](const PriceLevel& l) { return l.price == price; });
    if (lvlIt != levels.end()) {
        lvlIt->remove(slotIdx, pool_);
        if (lvlIt->empty()) levels.erase(lvlIt);
    }

    index_.erase(it);
    pool_.free(slotIdx);
    handler_.onOrderCancelled(OrderCancelled(req.orderId));
}

template<typename THandler>
size_t OrderBook<THandler>::buyDepth() const {
    uint32_t n = 0;
    for (size_t i = 0; i < buyLevels_.size(); ++i) n += buyLevels_[i].count;
    return n;
}

template<typename THandler>
size_t OrderBook<THandler>::sellDepth() const {
    uint32_t n = 0;
    for (size_t i = 0; i < sellLevels_.size(); ++i) n += sellLevels_[i].count;
    return n;
}

template<typename THandler>
const Order* OrderBook<THandler>::find(uint64_t orderId) const {
    typename std::unordered_map<uint64_t, uint32_t>::const_iterator it = index_.find(orderId);
    return it == index_.end() ? nullptr : &pool_[it->second].order;
}
