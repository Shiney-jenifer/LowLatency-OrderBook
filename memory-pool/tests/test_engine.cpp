/**
 * tests/test_engine.cpp
 *
 * Self-contained test suite. 
 */

#include "Engine.h"
#include "OrderBook.h"
#include "Messages.h"
#include <sstream>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <functional>

// ── Minimal test framework ────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

#define EXPECT_EQ(a, b) do {                                        \
    if ((a) == (b)) {                                               \
        ++g_passed;                                                 \
    } else {                                                        \
        ++g_failed;                                                 \
        std::cerr << "FAIL " << __FILE__ << ":" << __LINE__        \
                  << "  expected [" << (b) << "] got [" << (a)     \
                  << "]\n";                                         \
    }                                                               \
} while(0)

#define EXPECT_TRUE(x)  EXPECT_EQ((x), true)
#define EXPECT_FALSE(x) EXPECT_EQ((x), false)

// ── Helper: run a sequence of lines and capture stdout ───────────────────────

static std::string runLines(const std::vector<std::string>& lines) {
    std::ostringstream out;
    Engine engine(out);
    // Redirect cerr to /dev/null during the run so error messages don't
    // appear in the test output (they are expected for malformed inputs).
    std::streambuf* oldCerr = std::cerr.rdbuf();
    std::ostringstream errSink;
    std::cerr.rdbuf(errSink.rdbuf());
    for (auto& l : lines) engine.processLine(l);
    std::cerr.rdbuf(oldCerr);
    return out.str();
}

// ── Tests ─────────────────────────────────────────────────────────────────────

// Spec example verbatim
void test_specExample() {
    std::vector<std::string> input = {
        "0,1000000,1,1,1075",
        "0,1000001,0,9,1000",
        "0,1000002,0,30,975",
        "0,1000003,1,10,1050",
        "0,1000004,0,10,950",
        "BADMESSAGE",
        "0,1000005,1,2,1025",
        "0,1000006,0,1,1000",
        "1,1000004",
        "0,1000007,1,5,1025",
        "0,1000008,0,3,1050",
    };
    std::string expected =
        "2,2,1025\n"
        "4,1000008,1\n"
        "3,1000005\n"
        "2,1,1025\n"
        "3,1000008\n"
        "4,1000007,4\n";

    EXPECT_EQ(runLines(input), expected);
}

// A single buy that does not match
void test_noMatchBuy() {
    std::vector<std::string> input = {
        "0,1,1,5,1000",   // sell at 1000
        "0,2,0,3,999",    // buy at 999 — no cross
    };
    EXPECT_EQ(runLines(input), "");

    // Both orders should rest in the book
    std::ostringstream out;
    Engine eng(out);
    eng.processLine("0,1,1,5,1000");
    eng.processLine("0,2,0,3,999");
    EXPECT_EQ(eng.book().sellDepth(), (size_t)1);
    EXPECT_EQ(eng.book().buyDepth(),  (size_t)1);
}

// Aggressive buy exactly fills one resting sell
void test_exactFill() {
    std::string result = runLines({
        "0,10,1,5,100",   // sell 5 @ 100
        "0,11,0,5,100",   // buy  5 @ 100 — exact fill
    });
    EXPECT_EQ(result,
        "2,5,100\n"
        "3,11\n"
        "3,10\n");
}

// Aggressive buy partially fills one resting sell (aggressor leftover rests)
void test_partialFillAggressor() {
    std::string result = runLines({
        "0,10,1,3,100",   // sell 3 @ 100
        "0,11,0,5,100",   // buy  5 @ 100 — 3 filled, 2 left
    });
    EXPECT_EQ(result,
        "2,3,100\n"
        "4,11,2\n"
        "3,10\n");

    // aggressor (11) should be resting with qty 2
    std::ostringstream out;
    Engine eng(out);
    eng.processLine("0,10,1,3,100");
    eng.processLine("0,11,0,5,100");
    const Order* o = eng.book().find(11);
    EXPECT_TRUE(o != nullptr);
    EXPECT_EQ(o->quantity, (uint32_t)2);
}

// Aggressive sell partially fills one resting buy (resting leftover stays)
void test_partialFillResting() {
    std::string result = runLines({
        "0,10,0,5,100",   // buy  5 @ 100
        "0,11,1,3,100",   // sell 3 @ 100 — aggressor fully filled, resting reduced
    });
    EXPECT_EQ(result,
        "2,3,100\n"
        "3,11\n"
        "4,10,2\n");
}

// Multiple resting orders at same price matched in FIFO order
void test_fifoAtSamePrice() {
    std::string result = runLines({
        "0,1,1,2,100",    // sell 2 @ 100 (oldest)
        "0,2,1,2,100",    // sell 2 @ 100 (newer)
        "0,3,0,3,100",    // buy  3 @ 100 — should match order 1 first (fully), then order 2 partially
    });
    EXPECT_EQ(result,
        "2,2,100\n"
        "4,3,1\n"
        "3,1\n"
        "2,1,100\n"
        "3,3\n"
        "4,2,1\n");
}

// Better prices matched before worse prices
void test_pricePriority() {
    std::string result = runLines({
        "0,1,1,5,102",    // sell 5 @ 102
        "0,2,1,5,100",    // sell 5 @ 100  ← better (lower) for buyer
        "0,3,0,5,105",    // buy  5 @ 105  → should hit order 2 first
    });
    EXPECT_EQ(result,
        "2,5,100\n"
        "3,3\n"
        "3,2\n");
}

// Cancel removes a resting order so it is never matched
void test_cancelPreventsMatch() {
    std::ostringstream out;
    Engine eng(out);
    eng.processLine("0,1,1,5,100");  // resting sell
    eng.processLine("1,1");          // cancel it
    EXPECT_EQ(eng.book().sellDepth(), (size_t)0);
    eng.processLine("0,2,0,5,100");  // buy — nothing to match
    EXPECT_EQ(out.str(), "");        // no trades
    EXPECT_EQ(eng.book().buyDepth(), (size_t)1);
}

// Sell side aggressor sweeps through multiple buy levels
void test_sellSweepMultipleLevels() {
    std::string result = runLines({
        "0,1,0,3,105",    // buy  3 @ 105
        "0,2,0,3,103",    // buy  3 @ 103
        "0,3,1,6,100",    // sell 6 @ 100 → hits 105 first, then 103
    });
    EXPECT_EQ(result,
        "2,3,105\n"
        "4,3,3\n"
        "3,1\n"
        "2,3,103\n"
        "3,3\n"
        "3,2\n");
}

// Malformed messages do not crash and produce no output
void test_malformedMessages() {
    std::string result = runLines({
        "BADMSG",
        "0,abc,0,5,100",
        "0,1,0,-1,100",
        "0,2,0,5",        // too few fields
        "1",              // cancel with no id
        "0,3,0,5,100",    // valid — should still work after bad messages
    });
    EXPECT_EQ(result, "");  // no output (no trades triggered by the valid add)
}

// Duplicate orderId is rejected
void test_duplicateOrderId() {
    std::ostringstream out;
    Engine eng(out);
    eng.processLine("0,99,1,5,100");
    eng.processLine("0,99,0,5,100");  // duplicate — should log error, not crash
    EXPECT_EQ(eng.book().sellDepth(), (size_t)1);
    EXPECT_EQ(eng.book().buyDepth(),  (size_t)0);
}

// Aggressive order with leftover becomes resting and can be cancelled
void test_aggressorRestsAndCancels() {
    std::ostringstream out;
    Engine eng(out);
    eng.processLine("0,1,1,2,100");   // sell 2 @ 100
    eng.processLine("0,2,0,5,100");   // buy 5 @ 100 — 2 filled, 3 left
    EXPECT_EQ(eng.book().buyDepth(), (size_t)1);
    const Order* o = eng.book().find(2);
    EXPECT_TRUE(o != nullptr);
    EXPECT_EQ(o->quantity, (uint32_t)3);
    eng.processLine("1,2");           // cancel the resting aggressor
    EXPECT_EQ(eng.book().buyDepth(), (size_t)0);
}

// ── Runner ────────────────────────────────────────────────────────────────────

int main() {
    test_specExample();
    test_noMatchBuy();
    test_exactFill();
    test_partialFillAggressor();
    test_partialFillResting();
    test_fifoAtSamePrice();
    test_pricePriority();
    test_cancelPreventsMatch();
    test_sellSweepMultipleLevels();
    test_malformedMessages();
    test_duplicateOrderId();
    test_aggressorRestsAndCancels();

    std::cout << "\n=== Results: " << g_passed << " passed, "
              << g_failed << " failed ===\n";
    return g_failed == 0 ? 0 : 1;
}
