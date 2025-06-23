# STISLA: Self-Tuning Interpolation Search with Learning Anchors

## Abstract

Interpolation search is a considerably faster alternative to binary search for regularly distributed ordered arrays, with average-case time of O(log log n) against binary search's O(log n). Its performance advantage relies on the assumption of uniformly distributed data. In practice on real data with non-uniform distributions—such as exponential increase rates, clumped data, or power-law distributions—interpolation search generally falls back on linear-time efficiency due to errant position estimates.

We present **STISLA** (Self-Tuning Interpolation Search with Learning Anchors), a new adaptive search algorithm that preserves interpolation search's theoretical optimality but overcomes its empirical limitations on irregular data distributions. STISLA uses a dynamic anchor learning system that constructs a customized interpolation model during search time. Instead of depending only on array endpoints for linear interpolation, STISLA uses a learned table of critical anchor points that refine prediction quality progressively.

The algorithm runs at two stages: a first interpolation prediction based on the nearest trained anchors, and a localized binary search from a tolerance-bounded region. Upon prediction error surpassing a tunable threshold, STISLA progressively adds new anchor points in an adaptive manner, forming a more and more precise interpolation model of a data distribution's unique characteristics.

Our system shows significant performance gains against classical binary search and typical interpolation search on a range of data distributions. On regular distributions, we achieve 7× speedup against binary search with learning of minimal anchor overhead. Importantly, on difficult non-uniform data sets for which standard interpolation search does not work, STISLA exhibits steady performance gains by refining its interpolation model based on the data structure. STISLA connects the mathematical beauty of interpolation search with the application needs of realistic data access patterns. Due to its self-tuning ability, the algorithm is most appropriate for applications of repeated lookups on large sorted data collections with distribution characteristics either being unknown or changing over time, for example, database indexes, time series, and large data retrieval systems.


**Keywords:** Search algorithms, interpolation search, adaptive data structures, learned indices, algorithm optimization

---

```
STISLA Performance Benchmark
Array size: 1000000 elements
Lookup operations: 2000000

Phase 1: Learning anchor points...
Learned 2 anchor points

Phase 2: Performance measurement...
Results:
  Binary search: 87.2 ns/operation
  STISLA search: 7.5 ns/operation
  Performance:   11.69x faster (2 anchors learned)
```

