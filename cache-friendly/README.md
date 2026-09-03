# Order Book Matching Engine (Cache-Friendly, C++14)

A limit-order-book matching engine using contiguous vectors and integer-tick
pricing, designed for cache efficiency and clarity.

## Overview

Orders are organized into price levels, with each side kept sorted so the best
bid/ask is always at the front. Resting orders at a price are held in arrival
order (FIFO). Matching follows price-time priority: the best price trades first,
and among equal prices the oldest order trades first.

## Features

- Price-time priority matching (add, cancel, partial and full fills)
- Cache-friendly design using contiguous vectors and integer-tick pricing
- Best bid/ask found in O(1); new price levels inserted via binary search
- Robust input handling (malformed messages are skipped, never crash)

## Structure

- `include/` — header files (Order.h, OrderBook.h)
- `src/` — source files (OrderBook.cpp, main.cpp)
- `tests/` — test cases

## Build

    g++ -std=c++14 -O2 -Iinclude src/OrderBook.cpp src/main.cpp -o engine

## Run

    ./engine < input.txt

## Input format

    0,orderId,side,quantity,price    (add;  side 0 = buy, 1 = sell)
    1,orderId                        (cancel)

## Output format

    2,quantity,price                 (trade)
    3,orderId                        (order fully filled)
    4,orderId,remainingQuantity      (order partially filled)