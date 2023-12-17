## Introduction

This is the thrid version of Unordered Dictionary Benchmark (udb3). It
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

The following figure shows the results measured on an M1 MacBook Pro in
December, 2023.

<img align="left" src="__logs/231216-M1.png"/>

**Rationale.** 
