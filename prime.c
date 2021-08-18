#include <stdio.h>
#include <stdlib.h>

// Return integer square root of n.
unsigned int isqrt(unsigned int n)
{
    unsigned int root = 0;
    for (unsigned int pow4 = 1 << 30; pow4; pow4 >>= 2)
    {
        unsigned int limit = root + pow4;
        root >>= 1;
        if (n >= limit)
        {
            n -= limit;
            root += pow4;
        }
    }
    return root;
}

// Test if n is prime. If so return n, otherwise return the first identified
// factor of n (which may not be the smallest factor).
// Since all primes greater than 3 are in the form 6k +/- 1, we only test those
// potential factors, up to the square root of n.
// For the worst case n = 429467291 this algorithm will perform 21846 divisions
// looking for a factor.
unsigned int prime(unsigned int n)
{
    if (n <= 3) return n;
    if (!(n % 2)) return 2;
    if (!(n % 3)) return 3;

    unsigned int max = isqrt(n) + 1;
    for (unsigned int d = 5, o = 2;  d <= max; d += o, o = 6 - o) if (!(n % d)) return d;
    return n;
}

#define isprime(n) (prime(n) == (n))

int main(int argc, char *argv[])
{
    unsigned int n = (argc > 1) ? atoi(argv[1]) : 4294967291; // default use the worst case
    unsigned int f = prime(n);

    printf("isqrt(%u) = %u\n", n, isqrt(n));
    printf("First factor of %u is %u\n", n, f);
    printf("%u %s prime\n", n, (n == f) ? "is" : "is not");
    return 0;
}
