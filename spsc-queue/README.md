# Order Book Matching Engine (Low-Latency, C++14)

A limit-order-book matching engine built for low-latency, with a lock-free
producer/consumer pipeline feeding a single-threaded matcher.

## Overview

Orders are read and parsed on a producer thread, passed through a lock-free
SPSC (single-producer, single-consumer) queue, and matched on a single
consumer thread. Keeping the matcher single-threaded preserves deterministic
price-time priority while decoupling it from I/O.

## Features

- Price-time priority matching (add, cancel, partial and full fills)
- Cache-friendly design using contiguous vectors and integer-tick pricing
- Lock-free SPSC queue with acquire/release memory ordering
- Robust input handling (malformed messages are skipped, never crash)

## Structure

- `include/` — header files
- `src/` — source files
- `tests/` — test cases

## Build

    g++ -std=c++14 -O2 -pthread src/main.cpp src/OrderBook.cpp -o engine

## Run

    ./engine < input.txt

## Input format

    0,orderId,side,quantity,price    (add;  side 0 = buy, 1 = sell)
    1,orderId                        (cancel)

## Output format

    2,quantity,price                 (trade)
    3,orderId                        (order fully filled)
    4,orderId,remainingQuantity      (order partially filled)