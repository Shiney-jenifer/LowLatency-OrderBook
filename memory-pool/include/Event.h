#pragma once
#include "Order.h"
#include <cstdint>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

/**
 * Event.h  —  complete event system in ONE file
 *
 * BaseEvent           abstract base (all events inherit from it)
 * 7 derived events    one struct per event type
 * IEventHandler       pure virtual listener — one method per event
 * OutputHandler       concrete handler: formats events → stdout/stderr
 * MulticastHandler    fans one event out to multiple handlers
 */

// ── Base event ────────────────────────────────────────────────────────────────

struct BaseEvent {
    virtual ~BaseEvent() = default;
    virtual void print() const = 0;
};

// ── Derived event types ───────────────────────────────────────────────────────

struct OrderAccepted : BaseEvent {
    uint64_t orderId;
    Side     side;
    uint32_t quantity;
    double   price;
    OrderAccepted(uint64_t id, Side s, uint32_t qty, double p)
        : orderId(id), side(s), quantity(qty), price(p) {}
    void print() const override {
        std::cerr << "[ACCEPTED] orderId=" << orderId << "\n";
    }
};

struct OrderRejected : BaseEvent {
    uint64_t    orderId;
    const char* reason;
    OrderRejected(uint64_t id, const char* r) : orderId(id), reason(r) {}
    void print() const override {
        std::cerr << "[REJECTED] orderId=" << orderId << " reason=" << reason << "\n";
    }
};

struct OrderCancelled : BaseEvent {
    uint64_t orderId;
    OrderCancelled(uint64_t id) : orderId(id) {}
    void print() const override {
        std::cerr << "[CANCELLED] orderId=" << orderId << "\n";
    }
};

struct OrderCancelRejected : BaseEvent {
    uint64_t    orderId;
    const char* reason;
    OrderCancelRejected(uint64_t id, const char* r) : orderId(id), reason(r) {}
    void print() const override {
        std::cerr << "[CANCEL_REJECTED] orderId=" << orderId << " reason=" << reason << "\n";
    }
};

struct TradeExecuted : BaseEvent {
    uint32_t quantity;
    double   price;
    TradeExecuted(uint32_t qty, double p) : quantity(qty), price(p) {}
    void print() const override {
        std::cerr << "[TRADE] qty=" << quantity << " price=" << price << "\n";
    }
};

struct OrderFullyFilled : BaseEvent {
    uint64_t orderId;
    OrderFullyFilled(uint64_t id) : orderId(id) {}
    void print() const override {
        std::cerr << "[FULLY_FILLED] orderId=" << orderId << "\n";
    }
};

struct OrderPartiallyFilled : BaseEvent {
    uint64_t orderId;
    uint32_t remainingQuantity;
    OrderPartiallyFilled(uint64_t id, uint32_t rem)
        : orderId(id), remainingQuantity(rem) {}
    void print() const override {
        std::cerr << "[PARTIALLY_FILLED] orderId=" << orderId
                  << " remaining=" << remainingQuantity << "\n";
    }
};

// ── IEventHandler — one virtual method per event ─────────────────────────────

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void onOrderAccepted       (const OrderAccepted&)        = 0;
    virtual void onOrderRejected       (const OrderRejected&)        = 0;
    virtual void onOrderCancelled      (const OrderCancelled&)       = 0;
    virtual void onOrderCancelRejected (const OrderCancelRejected&)  = 0;
    virtual void onTradeExecuted       (const TradeExecuted&)        = 0;
    virtual void onOrderFullyFilled    (const OrderFullyFilled&)     = 0;
    virtual void onOrderPartiallyFilled(const OrderPartiallyFilled&) = 0;
};

// ── OutputHandler — formats events to wire output ────────────────────────────

class OutputHandler : public IEventHandler {
public:
    explicit OutputHandler(std::ostream& out) : out_(out) {}

    void onOrderAccepted       (const OrderAccepted&)         override { /* silent */ }
    void onOrderCancelled      (const OrderCancelled&)        override { /* silent */ }
    void onOrderRejected       (const OrderRejected& e)       override {
        std::cerr << "ERROR: order "  << e.orderId << " rejected: " << e.reason << "\n";
    }
    void onOrderCancelRejected (const OrderCancelRejected& e) override {
        std::cerr << "ERROR: cancel " << e.orderId << " rejected: " << e.reason << "\n";
    }
    void onTradeExecuted(const TradeExecuted& e) override {
        out_ << "2," << e.quantity << "," << fmtPrice(e.price) << "\n";
    }
    void onOrderFullyFilled(const OrderFullyFilled& e) override {
        out_ << "3," << e.orderId << "\n";
    }
    void onOrderPartiallyFilled(const OrderPartiallyFilled& e) override {
        out_ << "4," << e.orderId << "," << e.remainingQuantity << "\n";
    }

private:
    std::ostream& out_;

    static std::string fmtPrice(double price) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << price;
        std::string s = oss.str();
        auto dot = s.find('.');
        if (dot != std::string::npos) {
            size_t last = s.find_last_not_of('0');
            s.erase(last + 1);
            if (!s.empty() && s.back() == '.') s.pop_back();
        }
        return s;
    }
};

