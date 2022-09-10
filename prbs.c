#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

// PRBS pseudo-random bitstream generator and polynomial tester.
// Build with: CFLAGS="-Wall -Werror" make prbs

// Various known working polynomials. We define them here in psuedo-mathematical fashion to make transcription easier,
// but since they are shifted right before use the "+ 1" is irrelevant.
#define x(n) (1<<(n))
uint32_t polynomials[] =
{
    x(3) + x(2) + 1,
    x(4) + x(3) + 1,
    x(5) + x(4) + x(3) + x(2) + 1,
    x(6) + x(5) + x(3) + x(2) + 1,
    x(7) + x(6) + 1,                    // PBRS7
    x(7) + x(6) + x(5) + x(4) + 1,
    x(8) + x(6) + x(5) + x(4) + 1,
    x(9) + x(5) + 1,                    // PBRS9
    x(9) + x(4) + 1,
    x(9) + x(8) + x(6) + x(5) + 1,
    x(10) + x(7) + 1,
    x(10) + x(9) + x(7) + x(6) + 1,
    x(11) + x(9) + 1,                   // PBRS11
    x(11) + x(10) + x(9) + x(7) + 1,
    x(13) + x(12) + x(2) + x(1) + 1,
    x(13) + x(12) + x(10) + x(9) + 1,
    x(14) + x(13) + x(11) + x(9) + 1,
    x(15) + x(14) + 1,                  // PBRS15
    x(15) + x(14) + x(13) + x(11) + 1,
    x(16) + x(14) + x(13) + x(11) + 1,
    x(20) + x(3) + 1,                   // PBRS20
    x(23) + x(18) + 1,                  // PBRS23
    x(31) + x(28) + 1,                  // PBRS31
};

#define POLYNOMIALS (sizeof(polynomials)/sizeof(polynomials[0]))

#define usage() die("\
Usage:\n\
\n\
   prbs [-d] [index]\n\
\n\
Sequence PRBS polynomials and check for errors. If index 1 to %ld is specified just sequence that one, otherwise\n\
sequence them all. If -d is given, write the generated decimal sequence to stdout. All other info goes to stderr.\n", POLYNOMIALS)

int main(int argc, char **argv)
{
    int dump = 0, first = 1, last = POLYNOMIALS;

    if (*++argv && !strcmp(*argv, "-d"))
    {
        dump = 1;                   // dump generated values to stdout
        argv++;
    }

    if (*argv)
    {
        first=atoi(*argv);          // get a specific index
        if (first < 1 || first > POLYNOMIALS) usage(); // gave -h or something
        last=first;                 // just do the one
    }

    for (int index = first; index <= last; index++)
    {
        uint32_t polynomial = polynomials[index-1];
        uint32_t feedback = polynomial >> 1;                            // treat the polynomial as a bitmask
        uint32_t period = 0xffffffff >> __builtin_clz(feedback);        // a power of 2 - 1

        fprintf(stderr, "%d: polynomial 0x%X has period %u...\n", index, polynomial, period);

        uint32_t n = 1;                                                 // start at 1
        uint32_t iteration = 0;
        while (iteration < period)
        {
            if (dump) printf("%u ", n);
            if (!n || (iteration && n == 1)) break;                     // oops!
            n = ((n << 1) | __builtin_parity(n & feedback)) & period;   // advance to next
            iteration++;
        }

        if (dump) printf("\n");
        if (!n) die("Error, polynomial 0x%X sequences to 0 after %u iterations\n", polynomial, iteration);
        if (iteration < period) die("Error, polynomial 0x%X sequence repeats after %u iterations\n", polynomial, iteration);
        if (n != 1) die("Error, polynomial 0x%X sequence does not repeat after %u iterations (ends at %u)\n", polynomial, period, n);
    }
    return 0;
}
