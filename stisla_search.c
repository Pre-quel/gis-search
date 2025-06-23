/*  stisla_search.c  –  STISLA algorithm
 *
 *  Build:   clang -std=c17 -O3 -Wall -Wextra stisla_search.c -o stisla_search
 *  Usage:   ./stisla_search  [n_elems=1000000]  [lookups=2000000]
 *
 *  The program:
 *     1. builds a strictly increasing array <0,2,4,...>
 *     2. runs `lookups` random searches twice
 *        • pass #1 warms up (learns anchors)
 *        • pass #2 is timed and compared with classic binary search
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

/*----- tiny deterministic PRNG (xorshift64*) --------------------------------*/
static inline uint64_t rng64(uint64_t *s){
    uint64_t x=*s; x^=x>>12; x^=x<<25; x^=x>>27; *s=x;
    return x*0x2545F4914F6CDD1DULL;
}

/*------------- ANCHOR TABLE --------------------------------------------------*/
typedef struct { int64_t v; size_t i; } Anchor;

typedef struct {
    Anchor *a;
    size_t  n, cap;
} AnchorTable;

static int ensure_cap(AnchorTable *T, size_t need){
    if (need <= T->cap) return 0;
    size_t nc = T->cap? T->cap*2: 8;
    if (nc < need) nc = need;
    Anchor *na = realloc(T->a, nc*sizeof *na);
    if(!na){perror("realloc");return -1;}
    T->a = na; T->cap = nc; return 0;
}

/* binary search inside anchor table, returns left index */
static size_t anchor_lower(const AnchorTable *T, int64_t x){
    size_t lo=0, hi=T->n-1;
    while(lo+1<hi){
        size_t mid=(lo+hi)>>1;
        if(T->a[mid].v<=x) lo=mid; else hi=mid;
    }
    return lo;
}

/* insert anchor, keep sorted */
static int anchor_insert(AnchorTable *T, int64_t v, size_t i){
    if(ensure_cap(T,T->n+1)) return -1;
    size_t pos = anchor_lower(T,v)+1;
    for(size_t j=T->n;j>pos;--j) T->a[j]=T->a[j-1];
    T->a[pos]=(Anchor){.v=v,.i=i};
    T->n++; return 0;
}

/*-------------- STISLA --------------------------------------------------*/
static size_t stisla_search(const int64_t *arr, size_t n, int64_t key,
                          AnchorTable *T, size_t tol)
{
    /* fast‑path: array too small */
    if(n<32){
        for(size_t k=0;k<n;++k) if(arr[k]==key) return k;
        return SIZE_MAX;
    }

    /* Step 0 – ensure at least endpoints present */
    if(T->n==0){
        if(ensure_cap(T,2)) return SIZE_MAX;
        T->a[0]=(Anchor){arr[0],0};
        T->a[1]=(Anchor){arr[n-1],n-1};
        T->n=2;
    }

    /* Step 1 – find bounding anchors */
    size_t a_idx = anchor_lower(T,key);
    Anchor L=T->a[a_idx], R=T->a[a_idx+1];

    /* Step 2 – predict index by linear interpolation */
    size_t span = R.i - L.i;
    size_t pred;
    if(R.v == L.v) pred = L.i;              /* degenerate but safe */
    else{
        double frac = (double)(key - L.v) / (double)(R.v - L.v);
        if(frac<0.0) frac=0.0; else if(frac>1.0) frac=1.0;
        pred = L.i + (size_t)(span * frac);
    }

    /* Step 3 – local binary search within [lo,hi] */
    size_t lo = (pred>tol)? pred-tol : L.i;
    if(lo < L.i) lo = L.i;
    size_t hi = pred+tol;
    if(hi > R.i) hi = R.i;

    /* widen if too small – guarantees key is inside */
    if(arr[lo] > key) lo = L.i;
    if(arr[hi] < key) hi = R.i;

    while(lo<=hi){
        size_t mid = lo + ((hi-lo)>>1);
        int64_t v = arr[mid];
        if(v<key) lo = mid+1;
        else if(v>key) { if(mid==0) break; hi = mid-1; }
        else{
            /* Step 4 – learn: add anchor if prediction off */
            if( (pred>mid? pred-mid : mid-pred) > tol)
                anchor_insert(T,v,mid);
            return mid;
        }
    }
    /* not found – optional learning: insert anchor for miss */
    return SIZE_MAX;
}

/*----------------------- LIL' BENCHiE ----------------------------------------------*/
static inline uint64_t ns_now(void){
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts);
    return (uint64_t)ts.tv_sec*1000000000ull+ts.tv_nsec;
}

static size_t bin_search(const int64_t *a,size_t n,int64_t x){
    size_t lo=0, hi=n;
    while(lo<hi){
        size_t mid=lo+((hi-lo)>>1);
        int64_t v=a[mid];
        if(v<x) lo=mid+1;
        else if(v>x) hi=mid;
        else return mid;
    }
    return SIZE_MAX;
}

int main(int ac,char**av){
    size_t N = (ac>1)? strtoull(av[1],0,10): 1000000ull;
    size_t Q = (ac>2)? strtoull(av[2],0,10): 2000000ull;
    printf("elements=%zu  lookups=%zu\n",N,Q);

    int64_t *array = malloc(N*sizeof *array);
    if(!array){perror("malloc arr");return 1;}
    for(size_t i=0;i<N;++i) array[i] = (int64_t)(i<<1);   /* uniform 0,2,4,... */

    size_t *keys  = malloc(Q*sizeof *keys);
    if(!keys){perror("malloc keys");return 1;}
    uint64_t s=0xCAFEBABEDEADBEEFULL;
    for(size_t i=0;i<Q;++i)
        keys[i] = (size_t)(rng64(&s) % N);

    AnchorTable T={0};
    const size_t tol = 8;   /* prediction tolerance in indices */

    /* warm‑up */
    for(size_t i=0;i<Q;++i)
        (void)stisla_search(array,N,array[keys[i]],&T,tol);

    /* timed pass – PrIS */
    uint64_t t0=ns_now();
    volatile size_t sink=0;
    for(size_t i=0;i<Q;++i)
        sink ^= stisla_search(array,N,array[keys[i]],&T,tol);
    uint64_t pris_ns = ns_now()-t0;

    /* same queries – classic binary search */
    t0=ns_now();
    for(size_t i=0;i<Q;++i)
        sink ^= bin_search(array,N,array[keys[i]]);
    uint64_t bin_ns = ns_now()-t0;

    printf("binary: %.1f ns/op\n", (double)bin_ns/Q);
    printf("PrIS  : %.1f ns/op   (%.2f× faster, %zu anchors)\n",
           (double)pris_ns/Q, (double)bin_ns/pris_ns, T.n);

    /* avoid optimisation‑out */
    if(sink==0x123456) puts("");

    free(array); free(keys); free(T.a);
    return 0;
}
