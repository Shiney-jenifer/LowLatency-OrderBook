#include "OrderBook.h"
#include <iostream>
#include <algorithm>

void OrderBook::rest(const Order& order) {
   if (order.side == Side::Buy) {
        // bids_ : descending (high→low), comparison: lvl.price > p
        auto it = std::lower_bound(bids_.begin(), bids_.end(), order.price,
                [](const Level& lvl, int64_t p) { return lvl.price > p; });

         if (it != bids_.end() && it->price == order.price)
                it->orders.push_back(order);          // level exists
         else {
                Level lvl;
                lvl.price = order.price;
                lvl.orders.push_back(order);
                bids_.insert(it, lvl);                // new level
         }
   }
    else {
            // asks_ : ascending (low→high), comparison: lvl.price < p
            auto it = std::lower_bound(asks_.begin(), asks_.end(), order.price,
                    [](const Level& lvl, int64_t p) {return lvl.price < p;});

            if (it != asks_.end() && it->price == order.price)
                it->orders.push_back(order);
            else {
                    Level lvl;
                    lvl.price = order.price;
                    lvl.orders.push_back(order);
                    asks_.insert(it, lvl);
            }
        }
}

void OrderBook::match(Order& incoming, std::vector<Level>& oppSide)
{
    uint64_t locOrderId = 0;
    Level& level = oppSide[0];
    Order& resting = level.orders[0];
    uint32_t tradeQty = std::min(incoming.quantity, resting.quantity);

    out_ << "2," << tradeQty << "," << level.price << "\n";
    incoming.quantity -= tradeQty;
    resting.quantity -= tradeQty;

    if (incoming.quantity == 0)
    {
        out_ << "3," << incoming.orderId << "\n";

    }
    else
    {
        out_ << "4," << incoming.orderId << "," << incoming.quantity << "\n";
    }
    locOrderId = resting.orderId;
    if (resting.quantity == 0)
    {
        out_ << "3," << locOrderId << "\n";
        level.orders.erase(level.orders.begin());
        if (level.orders.empty())
        {
            oppSide.erase(oppSide.begin());
        }
    }
    else
    {
        out_ << "4," << locOrderId << "," << resting.quantity << "\n";
    }
}

void OrderBook::add(const Order& order) {
    Order incoming = order;
    

    if (incoming.side == Side::Buy) {
        while (incoming.quantity > 0 && !asks_.empty()
            && asks_[0].price <= incoming.price)
        {
            match(incoming,asks_);
        }
           
    }
    else 
    {
        while (incoming.quantity > 0 && !bids_.empty()
            && bids_[0].price >= incoming.price)
        {
            match(incoming, bids_);
        }
        
    }

    if (incoming.quantity > 0)
        rest(incoming);
}

bool OrderBook::cancelFrom(std::vector<Level>& lvlside, uint64_t orderId)
{
    for (size_t i = 0; i < lvlside.size(); ++i) {
        auto it = std::find_if(lvlside[i].orders.begin(), lvlside[i].orders.end(),
            [orderId](const Order& o) { return o.orderId == orderId; });
        if (it != lvlside[i].orders.end()) {
            lvlside[i].orders.erase(it);
            if (lvlside[i].orders.empty())
                lvlside.erase(lvlside.begin() + i);
            return true;
        }
    }
    return false;
}

void OrderBook::cancel(uint64_t orderId) {
    if (cancelFrom(bids_, orderId)) return;
    cancelFrom(asks_, orderId);
}