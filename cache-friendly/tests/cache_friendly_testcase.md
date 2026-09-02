# Test Case — Cache-Friendly Order Book

This test exercises price-time priority, partial and full fills, cancellation,
and malformed-input handling.

## Input (feed on stdin)

```
0,1000000,1,1,1075
0,1000001,0,9,1000
0,1000002,0,30,975
0,1000003,1,10,1050
0,1000004,0,10,950
BADMESSAGE
0,1000005,1,2,1025
0,1000006,0,1,1000
1,1000004
0,1000007,1,5,1025
0,1000008,0,3,1050
```

## Expected output (stdout)

```
2,2,1025
4,1000008,1
3,1000005
2,1,1025
3,1000008
4,1000007,4
```

## What it checks

- Multiple resting orders build up on both sides with no crossing (lines 1–8).
- `BADMESSAGE` is skipped and logged to stderr — the engine does not crash.
- The cancel (`1,1000004`) removes a resting buy.
- The final buy (`0,1000008,0,3,1050`) crosses the spread and sweeps the two
  sells resting at 1025:
  - trades 2 @ 1025 against order 1000005 (fully filled), buyer partially filled,
  - trades 1 @ 1025 against order 1000007 (partially filled), buyer fully filled.
- Trades execute at the **resting** price (1025), demonstrating price-time
  priority: the cheaper 1025 level is taken before 1050, and the older order
  (1000005) before the newer (1000007).

## How to run

    ./engine < input.txt

Redirect stderr away to see only trade output:

    ./engine < input.txt 2>/dev/null
