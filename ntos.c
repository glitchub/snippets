#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Given a 32-bit unsigned int, a base from 2 to 16, and a pointer to a string buffer of at least 33 bytes, format the
// number to the string.

// For each base, the initial divisor is B ^ (N-1), where B is the base and N is the maximum number of digits that can
// be represented in that base. The divisors below were generated with the following bash command:
//
// > for B in {2..16}; do M=$(bc <<<"obase=$B; (2^32)-1"); printf "%2d: divisor = %10d, max output = %s\n" $B $((B**(${#M}-1))) $M; done
//  2: divisor = 2147483648, max output = 11111111111111111111111111111111
//  3: divisor = 3486784401, max output = 102002022201221111210
//  4: divisor = 1073741824, max output = 3333333333333333
//  5: divisor = 1220703125, max output = 32244002423140
//  6: divisor = 2176782336, max output = 1550104015503
//  7: divisor = 1977326743, max output = 211301422353
//  8: divisor = 1073741824, max output = 37777777777
//  9: divisor = 3486784401, max output = 12068657453
// 10: divisor = 1000000000, max output = 4294967295
// 11: divisor = 2357947691, max output = 1904440553
// 12: divisor =  429981696, max output = 9BA461593
// 13: divisor =  815730721, max output = 535A79888
// 14: divisor = 1475789056, max output = 2CA5B7463
// 15: divisor = 2562890625, max output = 1A20DCD80
// 16: divisor =  268435456, max output = FFFFFFFF

char *ntos(uint32_t number, int base, char *string)
{
    const uint32_t divisors[] =
    {
        2147483648,
        3486784401,
        1073741824,
        1220703125,
        2176782336,
        1977326743,
        1073741824,
        3486784401,
        1000000000,
        2357947691,
        429981696,
        815730721,
        1475789056,
        2562890625,
        268435456
    };

    if (base < 2 || base > 16) base = 10;

    uint32_t divisor = divisors[base-2];

    while (divisor > 1 && divisor > number) divisor /= base;

    char *s = string;
    while (divisor)
    {
        *s++ = "0123456789ABCDEF" [(number / divisor) % base];
        number %= divisor;
        divisor /= base;
    }
    *s = 0;

    return string;
}

int main(int argc, char *argv[])
{
    char string[33];

    // first sanity check that all bases produce the expected result
    const char *results[] =
    {
        "11111111111111111111111111111111",
        "102002022201221111210",
        "3333333333333333",
        "32244002423140",
        "1550104015503",
        "211301422353",
        "37777777777",
        "12068657453",
        "4294967295",
        "1904440553",
        "9BA461593",
        "535A79888",
        "2CA5B7463",
        "1A20DCD80",
        "FFFFFFFF"
    };

    for (int base = 2; base <= 16; base++)
    {
        ntos(-1, base, string);
        if (strcmp(string, results[base - 2])) printf("Warning, number %u in base %u produced '%s', expected '%s'\n", -1, base, string, results[base-2]);
    }

    // now show specified or default test case

    // use default or specified numbers
    unsigned int number = 12345678;
    if (argc > 1) number=atoi(argv[1]);

    int base = 10;
    if (argc > 2) base=atoi(argv[2]);

    printf("Number %u in base %u is '%s'\n", number, base, ntos(number, base, string));
    return 0;
}
