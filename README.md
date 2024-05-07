## Introduction

This is the third version of Unordered Dictionary Benchmark (udb3). It
evaluates the performance of several popular hash table libraries in C or C++.
Results first; explanations later:

<img align="left" src="https://i.ibb.co/64nbn8k/240506-fast.png?v4"/>

### The tasks

There are two tasks. **The first focuses on insertion only**. We insert 80
million 32-bit integers, with duplicates, to the hash table and counts the
occurrence of each distinct key. There are 16.6 million entries left in the
table in the end. **The second task evaluates both insertions and deletions**.
The input is the same 80 million integers. We insert an integer to the hash
table if it is not in the table already, or delete it if present. There are 9.2
million integers left in the table. For both tasks, we record CPU time and peak
memory at 11 checkpoints and report the average CPU time per million inputs and
the average memory per entry in the table.

### The rationale behind the design

A hash table library can usually be made faster by lowering its default [load
factor][lf] threshold but this comes at a cost of memory. Controlling the
threshold would not give us a complete picture, either, because the optimal
threshold of a library depends on its implementation details and even at the
same load factor threshold, some libraries may use more auxiliary data than
others and thus take more memory.

In this benchmark, we **measure the speed and
the memory at the same time**. If library *A* is faster and uses less memory than
*B*, *A* will be better no matter how we tune the load factor of *B*. If *A* is
faster but uses more memory, we cannot draw a firm conclusion but at least we
will see the speed-memory tradeoff of a library.

## Results

The figure above shows the results measured on an M1 MacBook Pro in
December, 2023. Roughly speaking, a hash table library is represented by a
cloud of points. The relative positions of clouds inform their performance.

In the figure, `boost ensemble`, which implements an ensemble of hash tables in
the user code, is the fastest implementation using comparable memory to others. It is
**the clear winner in this benchmark**. `khashl ensemble` uses the least amount of
memory perhaps because it uses `realloc` and only doubles the memory during
rehashing; other libraries need to triple the memory.

Among the implementations without ensembles
[`boost::unordered_flat_map`][boost] is the fastest though it uses more
memory on the deletion workload. Built on top of [`absl::flat_hash_map`][absl],
[`phmap::flat_hash_map`][phmap] has impressive performance.
[`robin_hood::unordered_map`][rh] is a good choice if you need a small, fast and
portable hash table library. [`ska::bytell_hash_map`][ska] is a worthy mention
even though it has not been updated since 2018.

Among hash table libraries in C, [verstable][vt] is fast, lightweight and convenient
to use. It easily gets a recommendation from me. [M\*LIB][mlib] is fast on
insertion but does not perform well on deletion. I developed [khashl][khashl].

### On other hash table libraries

The figure above focuses on high-performance hash table libraries. The figure
below shows several other popular libraries.

<img align="left" src="https://i.ibb.co/BPmT54b/240506-slow.png?v4"/>

We can see that `std::unordered_map` in clang and [uthash][uthash] are times slower and
use more memory. They should be avoided if you need a fast hash table.
The developer of robin-hood-hashing now recommends his new
[`ankerl::unordered_dense`][ud]. However, this new library does not perform
well on this benchmark. In addition, its relative performance to
robin-hood-hashing also varies greatly, from 20% slower to >100% slower, on
different CPUs I have access to. If you want to use unordered\_dense in your
production code, you should compare it to other libraries on your specific
applications.

## Concluding Remarks

This benchmark only evaluates small key-value pairs. The performance on large
key-value pairs may be different. In addition, this benchmark mixes insertion,
update and deletion operations, but a library fast on one type of operation is
not necessarily fast on another. Depending on the frequency of each type of
operation, the results may look different, too. Overall, hash table evaluation
is hard. Developers interested in hash table performances are recommended to
read other benchmarks such as [this][bench1] and [this][bench2].

[lf]: https://en.wikipedia.org/wiki/Hash_table#Load_factor
[khashl]: https://github.com/attractivechaos/klib/blob/master/khashl.h
[boost]: http://bannalia.blogspot.com/2022/11/inside-boostunorderedflatmap.html
[absl]: https://abseil.io/docs/cpp/guides/container
[rh]: https://github.com/martinus/robin-hood-hashing
[ud]: https://github.com/martinus/unordered_dense
[ska]: https://github.com/skarupke/flat_hash_map
[vt]: https://github.com/JacksonAllan/Verstable
[mlib]: https://github.com/P-p-H-d/mlib
[uthash]: https://troydhanson.github.io/uthash/
[bench1]: https://martin.ankerl.com/2022/08/27/hashmap-bench-01/
[bench2]: https://github.com/renzibei/hashtable-bench
