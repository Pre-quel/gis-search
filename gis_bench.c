/* gis_bench.c  –  GIS vs. binary‑search micro‑benchmark */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdlib.h>

#define arc4random_uniform(n) ((size_t)(rand() % (n)))


#ifndef CLOCK_MONOTONIC
#error "clock_gettime unavailable – compile on macOS 10.12+ or another POSIX platform"
#endif

/*--------------------------------------------------*
 *  Classic iterative binary search (strictly inc.) *
 *--------------------------------------------------*/
static size_t bin_search(const int64_t *a, size_t n, int64_t x)
{
    size_t lo = 0, hi = n;
    while (lo < hi) {
        size_t mid = lo + ((hi - lo) >> 1);
        int64_t v = a[mid];
        if (v < x) lo = mid + 1;
        else if (v > x) hi = mid;
        else return mid;
    }
    return SIZE_MAX;
}

/*--------------------------------------------------*
 *  Galloping‑Interpolation Search                   *
 *--------------------------------------------------*/
static size_t gis_search(const int64_t *a, size_t n, int64_t x)
{
    if (!n || x < a[0] || x > a[n - 1]) return SIZE_MAX;

    size_t L = 0, step = 1;
    while (L + step < n && a[L + step] < x) { L += step; step <<= 1; }
    size_t R = (L + step < n) ? L + step : n - 1;

    while (R > L + 16) {
        if (a[R] == a[L]) break;
        double frac = (double)(x - a[L]) / (double)(a[R] - a[L]);
        size_t P = L + (size_t)((R - L) * frac);
        int64_t v = a[P];
        if      (v < x) L = P + 1;
        else if (v > x) R = P - 1;
        else return P;
    }
    for (size_t i = L; i <= R; ++i)
        if (a[i] == x) return i;
    return SIZE_MAX;
}

/*---- timing helper ----*/
static inline uint64_t ns_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(int argc, char *argv[])
{
    size_t n_elems  = (argc > 1) ? strtoull(argv[1], NULL, 10) : 1000000ull;
    size_t n_lookup = (argc > 2) ? strtoull(argv[2], NULL, 10) : 5000000ull;
    printf("elem=%zu  lookups=%zu\n", n_elems, n_lookup);

    /* Build strictly increasing array: a[i] = 2*i  */
    int64_t *a = malloc(n_elems * sizeof *a);
    if (!a) { perror("malloc"); return EXIT_FAILURE; }
    for (size_t i = 0; i < n_elems; ++i) a[i] = (int64_t)(i << 1);

    /* Prepare random indices to search (uniform distribution) */
    size_t *rnd_idx = malloc(n_lookup * sizeof *rnd_idx);
    if (!rnd_idx) { perror("malloc"); return EXIT_FAILURE; }
    for (size_t i = 0; i < n_lookup; ++i) rnd_idx[i] = (size_t)arc4random_uniform((uint32_t)n_elems);

    volatile size_t sink = 0; /* prevent optimisation‑out */

    /* --- Binary search timing --- */
    uint64_t t0 = ns_now();
    for (size_t i = 0; i < n_lookup; ++i)
        sink ^= bin_search(a, n_elems, a[rnd_idx[i]]);
    uint64_t t1 = ns_now();
    double ns_bin = (double)(t1 - t0) / (double)n_lookup;

    /* --- GIS timing --- */
    t0 = ns_now();
    for (size_t i = 0; i < n_lookup; ++i)
        sink ^= gis_search(a, n_elems, a[rnd_idx[i]]);
    t1 = ns_now();
    double ns_gis = (double)(t1 - t0) / (double)n_lookup;

    printf("binary: ns/op = %.1f\n", ns_bin);
    printf("GIS   : ns/op = %.1f  (%.2f× faster)\n",
           ns_gis, ns_bin / ns_gis);

    if (sink == 0xdeadbeef) puts("impossible");

    free(a);
    free(rnd_idx);
    return 0;
}
