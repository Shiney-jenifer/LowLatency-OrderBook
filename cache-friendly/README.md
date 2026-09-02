# Tests

Each test is a pair of files: an input fed to the engine on stdin, and the
expected output on stdout.

## Test 1 — Comprehensive (`test1_input.txt` / `test1_expected.txt`)

Exercises the full feature set: multiple resting orders on both sides, a
malformed message (`BADMESSAGE`) that is skipped, a cancel (`1,1000004`),
and a final aggressive buy that sweeps two price levels producing partial
and full fills.

## Test 2 — Simple full match (`test2_input.txt` / `test2_expected.txt`)

A single sell rests, then a matching buy fully fills it — both orders are
reported as fully filled.

## Running a test

    ./engine < test1_input.txt

Compare the output against the matching `_expected.txt` file. On Linux/Mac:

    ./engine < test1_input.txt | diff - test1_expected.txt && echo PASS
