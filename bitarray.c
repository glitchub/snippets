#include <string.h>
#include <stdlib.h>

#define BTYPE unsigned int            // or unsigned char on AVR for example
#define BWIDTH (sizeof(BTYPE)*8)
#define BMASK(bit) ((BTYPE)1<<((bit) & BWIDTH-1))

typedef struct
{
    unsigned bits;          // number of represented bits
    unsigned set;           // count of set bits
    BTYPE array[];          // zero length array
} bitarray;

// return 1 if numbered bit is set, 0 if clear, -1 if bit number is invalid
int bitarray_test(bitarray *b, unsigned bit)
{
    if (bit >= b->bits) return -1;
    return (b->array[bit/BWIDTH] & BMASK(bit)) ? 1 : 0;
}

// Set numbered bit and return 0, or -1 if bit number is invalid
// Track the number of bits set
int bitarray_set(bitarray *b, unsigned bit)
{
    if (bit >= b->bits) return -1;
    if (!bitarray_test(b, bit))
    {
        b->set++;
        b->array[bit/BWIDTH] |= BMASK(bit);
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
        b->array[bit/BWIDTH] &= ~BMASK(bit);
    }
    return 0;
}

// Clear the bitarray
void bitarray_init(bitarray *b)
{
    memset(b->array, 0, (b->bits + (BWIDTH-1)) / 8); // size in bytes
    b->set = 0;
}

// Invert the bitarray
void bitarray_invert(bitarray *b)
{
    for (int n = 0; n < (b->bits + (BWIDTH-1)) / BWIDTH; n++) b->array[n] ^= (BTYPE)-1;
    b->set = b->bits - b->set;
}

// Create new bit array and return pointer, or NULL if OOM.
// Caller must free() it when done.
bitarray *bitarray_create(unsigned bits)
{
    bitarray *b = malloc(sizeof(bitarray) + ((bits + (BWIDTH-1)) / 8)); // size in bytes
    if (b)
    {
        b->bits = bits;
        bitarray_init(b);
    }
    return b;
}

// Given a bit number, return that number if the bit is set, otherwise return
// the next highest set bit, or -1 if none.
int bitarray_next(bitarray *b, unsigned bit)
{
    while (bit < b->bits && b->array[bit/BWIDTH] < BMASK(bit)) bit = (bit & ~(BWIDTH-1)) + BWIDTH;
    while (bit < b->bits && !(b->array[bit/BWIDTH] & BMASK(bit))) bit++;
    return (bit < b->bits) ? bit : -1;
}

#ifdef POC  // Compiled with 'CFLAGS=-POC make bitarray'
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
    assert(bitarray_next(b, 201) == -1);
    assert(!bitarray_set(b, 252));
    assert(bitarray_set(b, 253));
    assert(b->set == 4);
    bitarray_invert(b);
    assert(b->set == 249);
    assert(bitarray_test(b, 99) == 0);
    printf("Seems to be working...\n");
}
#endif
