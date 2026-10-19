# LowLatency-OrderBook

A limit-order-book matching engine in C++20, exploring low-latency design.
Two matching-engine designs plus a lock-free queue that decouples order
ingestion from matching.

## Folders

- **cache-friendly/** — main design: contiguous vectors, integer-tick pricing, price-time priority.
- **memory-pool/** — earlier design using a memory pool, event system, and templates.
- **spsc-queue/** — lock-free single-producer/single-consumer queue feeding a single-threaded matcher.

## Design

- Price-time priority matching (add, cancel, partial/full fills)
- Integer-tick prices (`int64_t`), never floating point
- Cache-friendly contiguous storage
- Lock-free SPSC queue with acquire/release memory ordering
- Producer thread parses input; single consumer thread matches

## Input / Output

    Input:
      0,orderId,side,quantity,price   (add; side 0 = buy, 1 = sell)
      1,orderId                       (cancel)

    Output:
      2,quantity,price                (trade)
      3,orderId                       (fully filled)
      4,orderId,remainingQuantity     (partially filled)

Malformed input is skipped and logged to stderr; the engine never crashes.

## Build & Run

    g++ -std=c++20 -O2 -pthread <sources> -o engine
    ./engine < input.txt
