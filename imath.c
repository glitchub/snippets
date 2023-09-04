#include <stdlib.h>
#include <stdint.h>

// Return integer x to the nth power (or 0 on overflow)
uint64_t ipow(uint64_t x, uint32_t n)
{
    uint64_t pow = 1;
    if (!x || x >= (1L << 32)) return 0;
    if (!n) return 1;
    if (n == 1) return x;
    while(1)
    {
        if (n & 1 && __builtin_mul_overflow(pow, x, &pow)) return 0;
        if (!(n >>= 1)) return pow;
        if (__builtin_mul_overflow(x, x, &x)) return 0;
    }
}

// Return integer nth root of x (or 0 on error/overflow)
uint32_t iroot(uint64_t x, uint32_t n)
{
    if (!n || !x) return 0;
    if (n == x) return 1;
    if (n == 1 || x == 1) return x;

    uint64_t base = 1, root = 2;

    // Start with largest power of 2 less than the nth root
    while(1)
    {
        uint64_t r = ipow(root, n);
        if (r == x) return root; // stumbled upon
        if (!r || r > x) break;
        base = r;
        root <<= 1;
    }
    root >>= 1;

    for (uint32_t min = root >> 1; min && x > base; min >>= 1)
    {
        uint64_t r = ipow(root + min, n);
        if (r && r <= x)
        {
            root += min;
            base = r;
        }
    }
    return root;
}

// Return integer square root of x.
uint32_t isqrt(uint64_t x)
{
    if (x <= 1) return x;
    uint64_t root = 0;
    for (uint64_t pow4 = 1L << 62; pow4; pow4 >>= 2)
    {
        uint64_t base = root + pow4;
        root >>= 1;
        if (x >= base)
        {
            x -= base;
            root += pow4;
        }
    }
    return root;
}

// POC

#include <stdio.h>

int main(int argc, char *argv[])
{
    // expect two numeric arguments
    uint64_t x = strtoull(argv[1],NULL,0);
    uint32_t n = strtoul(argv[2],NULL,0);

    uint32_t s = isqrt(x);
    if (!s) printf("2√%llu error\n", x); else printf("2√%llu = %u, %u^2 = %llu\n", x, s, s, (uint64_t)s * s);

    uint32_t r = iroot(x, n);
    if (!r) printf("%u√%llu error\n", n, x); else printf("%u√%llu = %u, %u^%u = %llu\n", n, x, r, r, n, ipow(r, n));

    uint64_t p = ipow(x, n);
    if (!p) printf("%llu^%u error\n", x, n); else printf("%llu^%u = %llu, %u√%llu = %u\n", x, n, p, n, p, iroot(p, n));
}
