# Matching Engine

A C++14 in-memory limit-order-book matching engine.

---

## Build

```bash
# Build the engine binary and test binary
make all

# Or just the engine
make matching_engine
```

Requires `g++` with C++14 support (any GCC ≥ 7 on Linux).  No third-party
libraries are used in production code.

---

## Run

```bash
./matching_engine < data/dataset_01_spec_example.txt
```

Input is read from **stdin** (one message per line).  
Output (trades and fills) is written to **stdout**.  
Error diagnostics are written to **stderr**.

---

## Test

```bash
make test      # runs 22 unit tests
make check     # diffs engine output against expected_01.txt / expected_02.txt
```

---

## Project layout

```
src/
  Order.h          – Order struct and Side enum
  Messages.h/.cpp  – Input/output message types, parsing, formatting
  OrderBook.h/.cpp – Core matching engine data structure
  Engine.h/.cpp    – Orchestrates parsing → book → output
  main.cpp         – stdin/stdout entry point
tests/
  test_engine.cpp  – Self-contained test suite (no framework required)

```

---

## Design

### Data structures

Memory Pool using vectors.
unorederd-set.

### Complexity

| Operation | Complexity | Notes |
|---|---|---|
| `AddOrderRequest` (no match) | O(log P) | Single tree insert; P = distinct price levels |
| `AddOrderRequest` (k matches) | O(k log P) | k tree pops, each O(log P) when a level empties |
| `CancelOrderRequest` | O(log P) | Hash lookup O(1) + tree erase O(log P) |
| `OrderFullyFilled` removal | O(1) | List erase via stored iterator |

In practice P is bounded by real-world tick grids, so log P is effectively
constant.  The hash-map index is the key enabler for O(1) cancel — without it
a cancel would require an O(P) scan.

### Trade-offs



### Production enhancements

1. **Fixed-point price representation** – eliminates float comparison issues
   and allows using integer keys in the level map.

2. **Memory pool for list nodes** – avoids per-order heap allocations; a
   slab allocator with 64-byte-aligned nodes would improve cache locality
   during matching sweeps.

3. **Lock-free/sharded book** – for multi-threaded gateways, one lock-free
   MPSC queue per symbol feeding a single writer thread per book eliminates
   contention entirely.

4. **Sequence-number log** – persist every input message to a write-ahead log
   for crash recovery and market audit trails.

5. **Metrics / telemetry** – per-symbol latency histograms (order-to-ack,
   order-to-fill) exported via a stats socket.
