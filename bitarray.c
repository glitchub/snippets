#include <string.h>
#include <stdlib.h>

typedef struct
{
    unsigned bits;          // number of represented bits
    unsigned set;           // count of set bits
    unsigned char array[];  // zero length array
} bitarray;

// return 1 if numbered bit is set, 0 if clear, -1 if bit number is invalid
int bitarray_test(bitarray *b, unsigned bit)
{
    if (bit >= b->bits) return -1;
    return b->array[bit/8] & (1 << (bit & 7)) ? 1 : 0;
}

// Set numbered bit and return 0, or -1 if bit number is invalid
// Track the number of bits set
int bitarray_set(bitarray *b, unsigned bit)
{
    if (bit >= b->bits) return -1;
    if (!bitarray_test(b, bit))
    {
        b->set++;
        b->array[bit/8] |= (1 << (bit & 7));
    }
    return 0;
}

// Reset numbered bit and return 0, or -1 if bit number is invalid
// Track the number of bits set
int bitarray_clear(bitarray *b, unsigned bit)
{
    if (bit >= b->bits) return -1;
    if (bitarray_test(b, bit))
    {
        b->set--;
        b->array[bit/8] &= ~(1 << (bit & 7));
    }
    return 0;
}

// Clear all bits in the array
void bitarray_init(bitarray *b)
{
    memset(b->array, 0, (b->bits+7)/8);
    b->set = 0;
}

// Create new bit array and return pointer, or NULL if OOM.
// Caller must free() it when done.
bitarray *bitarray_create(unsigned bits)
{
    bitarray *b = malloc(sizeof(bitarray)+((bits+7)/8));
    if (b)
    {
        b->bits = bits;
        bitarray_init(b);
    }
    return b;
}

// Given a bit number, the next highest set bit (or that bit, if it's set).
// Or return -1 if none.
int bitarray_next(bitarray *b, unsigned bit)
{
    if (bit >= b->bits) return -1;
    if (b->array[bit/8] < 1 << (bit & 7))
    {
        bit = (bit & ~7) + 8;
        while (1)
        {
            if (bit >= b->bits) return -1;
            if (b->array[bit/8]) break;
            bit += 8;
        }
    }
    while (!(b->array[bit/8] & (1 << (bit & 7)))) bit++;
    return bit;
}

#if 1
#include <stdio.h>
#include <assert.h>
int main(void)
{
    bitarray *b = bitarray_create(253);
    assert(b->set == 0);
    assert(!bitarray_set(b, 44));
    assert(b->set == 1);
    assert(!bitarray_set(b, 99));
    assert(b->set == 2);
    assert(!bitarray_set(b, 200));
    assert(b->set == 3);
    assert(!bitarray_set(b, 44));
    assert(b->set == 3);            // should still be 3
    assert(!bitarray_clear(b, 38));
    assert(b->set == 3);            // should still be 3
    assert(bitarray_test(b, 99) == 1);
    assert(bitarray_set(b, 253));   // should fail
    assert(bitarray_test(b, 44) == 1);
    assert(!bitarray_clear(b,44));
    assert(bitarray_test(b, 44) == 0);
    assert(b->set == 2);
    assert(bitarray_next(b, 44) == 99);
    assert(bitarray_next(b, 98) == 99);
    assert(!bitarray_set(b, 98));
    assert(bitarray_next(b, 44) == 98);
    assert(bitarray_next(b, 98) == 98);
    assert(bitarray_next(b, 99) == 99);
    assert(bitarray_next(b, 100) == 200);
    printf("Seems to be working...\n");
}
#endif
