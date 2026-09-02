#pragma once
#include "OrderBook.h"
#include "Messages.h"
#include "Event.h"
#include <ostream>
#include <string>
#include <unordered_set>


class Engine {
public:
    explicit Engine(std::ostream& out)
        : outputHandler_(out)
        , book_(outputHandler_)
    {}

    bool processLine(const std::string& line);
    const OrderBook<OutputHandler>& book() const { return book_; }

private:
    OutputHandler              outputHandler_;
    OrderBook<OutputHandler>   book_;
    uint64_t                   seqNum_{0};
    std::unordered_set<uint64_t> knownIds_;
};
