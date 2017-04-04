NRPA
====

This is a parallel implementation of the NRPA algorithm (Nested Rollout Policy Allocation).

Build
=====

    cd src
    make

Run a test (same game)
======================

    cd src
    ./same

Advanced testing
================

Run all tests 

    make test

Show the average score curve (requires gnuplot)

    make run420
    make show420

How to use NRPA to solve a new game
===================================

1. Define a Board class and a Move class following as in leftmove.cpp
2. Include "nrpa.hpp"
3. In the main function, call:

    Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));
    
where:

- 'Board' is your board class
- 'Move' is your move class
- '5' is the maximum Nrpa depth + 1
- 'MaxPlayoutLength' is an upper bound on the number of moves in a single game
- 'MaxLegalMoves' is an upper bound on the number of distinct legal moves

4. Compile your program

5. run it.


Usage
=====

    Usage: ./same [options]
    where option is any of these: 
            --num-run=NUM, -r NUM
                    Number of test runs (default: 4).
            --num-level=NUM, -l NUM
                    Nrpa depth (default: 4).
            --num-thread=NUM, -x NUM
                    Number of threads (0 = hardware capacity, default: 0).
            --timeout=NUM, -t NUM
                    Timeout in sec. for a single run (0 = no timeout, default: 0).
            --iter-stats, -s
                    Enable iteration based statistics (default: no).
            --timer-stats, -S
                    Enable timer based statistics (default: no).
            --tag=STRING, -T STRING
                    Set a tag name for this run, it will be appened to statistics file name (default: None).
            --parallel-level=NUM, -p NUM
                    Go parallel call at level N (default: 1).
            --parallel-strat=NUM, -P NUM
                    Use parallelization strategy number N (default: 1).
            --thread-stats, -q
                    Enable thread statistics (default: 0).
            --help, -h
                    This help.
    
Authors
=======

Benjamin Negrevergne
Tristan Cazenave

License
=======

GPL3
