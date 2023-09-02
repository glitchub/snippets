#include <stdlib.h>

// Return x to the n power (or 0 on overflow)
unsigned long long ipow(unsigned int x, unsigned int n)
{
    unsigned long long xx = x, pow = 1;
    if (!x) return 0;
    if (!n) return 1;
    if (n == 1) return x;
    while(1)
    {
        if (n & 1 && __builtin_mul_overflow(pow, xx, &pow)) return 0;
        if (!(n >>= 1)) return pow;
        if (__builtin_mul_overflow(xx, xx, &xx)) return 0;
    }
}


// Return nth root of x (or 0 on error/overflow)
unsigned int iroot(unsigned long long x, unsigned int n)
{
    if (!n || !x) return 0;
    if (n == x) return 1;
    if (n == 1 || x == 1) return x;

    unsigned long long base = 1;
    unsigned long long root = 2;

    // find smallest power of 2 greater than x
    while(1)
    {
        unsigned long long r = ipow(root, n);
        if (r == x) return root; // stumbled upon
        if (!r || r > x) break;
        base = r;
        root <<= 1;
    }

    root >>= 1; // start with largest power of 2 less than x

    for (unsigned int bit = root >> 1; bit && x > base; bit >>= 1)
    {
        unsigned long long r = ipow(root + bit, n);
        if (!r) return 0; // shouldn't happen
        if (r <= x)
        {
            root += bit;
            base = r;
        }
    }
    return root;
}

// POC

#include <stdio.h>

int main(int argc, char *argv[])
{
    // expect two numeric arguments
    unsigned long long x = strtoull(argv[1],NULL,0);
    unsigned int n = strtoul(argv[2],NULL,0);

    unsigned int r = iroot(x, n);
    if (!r) printf("%u√%llu error\n", n, x); else printf("%u√%llu = %u, %u^%u = %llu\n", n, x, r, r, n, ipow(r, n));

    unsigned long long p = ipow(x, n);
    if (!p) printf("%llu^%u error\n", x, n); else printf("%llu^%u = %llu, %u√%llu = %u\n", x, n, p, n, p, iroot(p, n));
}
