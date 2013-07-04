insertionfinder
===============
Find insertions for Rubik's cube fewest moves.

Compile
-------
A Compiler supports C++11 is required.
For g++ version >= 4.7, use `g++ -std=c++11 -O3 -o insertionfinder src/\*.cpp -pthread` to compile

Run
---
For the first time, create a folder named Algfiles and run `insertionfinder --init`
use `insertionfinder -v` to verify a cube,
use `insertionfinder -s` to find insertions.
The input should contain 3 lines:
first line: the scramble
second line: your skeleton
third line: the algorithms (0x01-0xff)

Algorithms
----------
0x01 for corner 3-cycle
0x02 for corner 2x2-cycles
0x04 for corner twists
0x08 for edge 3-cycle
0x10 for edge 2x2-cycles
0x20 for edge flips
0x40 for other edge algorithms
0x80 for other algorithms
