## Introduction

This is the third version of Unordered Dictionary Benchmark (udb3). It
evaluates the performance of several popular hash table libraries in C or C++.
here are two tasks. **The first task focuses on insertion only**. We insert 80
million 32-bit integers, with duplicates, to the hash table and counts the
occurrence of each distinct key. There are 16.6 million entries left in the
table in the end. **The second task evaluates both insertions and deletions**.
The input is the same 80 million integers. We insert an integer to the hash
table if it is not in the table already, or delete it if present. There are 9.2
million integers left in the table. For both tasks, we record CPU time and peak
memory at 11 checkpoints, reports the average CPU time per million inputs and
the average memory per entry in the table, and plot them.

Note that a hash table library can usually be made faster by lowering its
default [load factor][lf] threshold but this comes at a cost of memory. It may
be tempted to evaluate libraries at the same load factor threshold. However,
the optimal threshold of a library depends on its implementation details and
even at the same threshold, some libraries may use more auxiliary data than
others and thus take more memory. Controlling the load factor would not give us
a complete picture, either. In this benchmark, we **measure speed and memory at
the same time**. If library *A* is faster and uses less memory than *B*, *A*
will be better no matter how we tune the load factor of *B*. If *A* is faster
but uses more memory, we cannot draw a firm conclusion but at least we will
see the speed-memory tradeoff of a library.

## Results

The following figure shows the results measured on an M1 MacBook Pro in
December, 2023. Roughly speaking, a hash table library is represented by a
cloud of points. The relative positions of clouds inform their performance.

<img align="left" src="__logs/231216-M1.png"/>

[lf]: https://en.wikipedia.org/wiki/Hash_table#Load_factor
