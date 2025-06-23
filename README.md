# STISLA: Self-Tuning Interpolation Search with Learning Anchors

## Abstract

Interpolation search represents a significant improvement over binary search for uniformly distributed sorted arrays, achieving O(log log n) average-case complexity compared to binary search's O(log n). However, this performance advantage critically depends on the assumption of uniform data distribution. When applied to real-world datasets with non-uniform distributions—such as exponential growth patterns, clustered data, or power-law distributions—interpolation search often degrades to linear-time performance due to poor position predictions.

We introduce **STISLA** (Self-Tuning Interpolation Search with Learning Anchors), a novel adaptive search algorithm that maintains the theoretical advantages of interpolation search while addressing its practical limitations on irregular data distributions. STISLA employs a dynamic anchor learning mechanism that builds a personalized interpolation model during search operations. Rather than relying solely on array endpoints for linear interpolation, STISLA maintains a learned table of strategic anchor points that improve prediction accuracy over time.

The algorithm operates in two phases: an initial interpolation prediction using the nearest learned anchors, followed by a localized binary search within a tolerance-bounded region. When prediction errors exceed a configurable threshold, STISLA adaptively inserts new anchor points, creating a progressively more accurate interpolation model tailored to the specific data distribution characteristics.

Our implementation demonstrates substantial performance improvements over both classical binary search and standard interpolation search across diverse data patterns. On uniform distributions, STISLA achieves up to 7× speedup over binary search while learning minimal anchor overhead. More significantly, on challenging non-uniform datasets where traditional interpolation search fails, STISLA maintains consistent performance gains by adapting its interpolation model to the underlying data structure.

STISLA bridges the gap between the theoretical elegance of interpolation search and the practical demands of real-world data access patterns. The algorithm's self-tuning nature makes it particularly suitable for applications involving repeated searches on large sorted datasets with unknown or evolving distribution characteristics, such as database indexing, time-series analysis, and large-scale data retrieval systems.

**Keywords:** Search algorithms, interpolation search, adaptive data structures, learned indices, algorithm optimization

