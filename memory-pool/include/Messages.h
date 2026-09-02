#pragma once
#include "Order.h"
#include <string>
#include <cstdint>

// ── Input messages ────────────────────────────────────────────────────────────

struct AddOrderRequest {
    uint64_t orderId;
    Side     side;
    uint32_t quantity;
    double   price;
};

struct CancelOrderRequest {
    uint64_t orderId;
};

enum class MessageType { Add, Cancel, Invalid };

struct InputMessage {
    MessageType      type;
    AddOrderRequest  add;
    CancelOrderRequest cancel;

    InputMessage() : type(MessageType::Invalid), add(), cancel() {}
};

// ── Parsing ───────────────────────────────────────────────────────────────────
// Returns message with type=Invalid on parse failure
InputMessage parseMessage(const std::string& line);
