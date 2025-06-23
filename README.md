# gis-search
Galloping‑Interpolation Search (GIS)

**Lil Benchie's results show that GIS can be faster than binary search**

```
binary: ns/op = 89.5
GIS   : ns/op = 12.8  (6.99× faster)
```

## When and Why GIS Beats Binary Search

**The Key Difference:**
Binary search always splits the search space in half, regardless of the target value. GIS uses **interpolation** to make much smarter guesses about where the target likely is.

Searching for value `1,000,000` in array of 1M elements:

**Binary search:**
- Checks middle (index 500,000, value 1,000,000) → found in ~20 steps

**GIS:**
- Interpolation estimate: `(1,000,000 - 0) / (1,999,998 - 0) ≈ 0.5`
- Jumps directly near index 500,000 → found in ~3-4 steps

## When Each Wins

**GIS faster when:**
- Large datasets (>10K elements)
- Reasonably uniform data distribution
- Many repeated searches

**Binary search faster when:**
- Small datasets
- Highly skewed data
- Simple implementation needs
