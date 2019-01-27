// Quick sort, requires around log2(n)*6 words of free stack, where 'n' is the
// number of elements to sort and 'word' is sizeof(void *). This generally
// rules out embedded systems, in that case use bubble sort.

// Given two pointers into a contiguous array of ints, recursively sort the
// elements between the pointers.
static void qs(int *first, int *last)
{
    if (first < last)
    {
        int *f=first, *l=last;
        while (1)
        {
            while (*f < *first) f++;
            while (*l > *first) l--;
            if (f >= l) break;
            // swap *f and *l
            *l^=*f; *f^=*l; *l--^=*f++;
        }
        qs(first, l);
        qs(l+1, last);
    }
}

#if 1
// POC, accept integers on command line and print sorted list, e.g.:
//   ./qs $(hexdump -n20000 -e '1/2 "%d\n"' /dev/urandom)
#include <stdio.h>
#include <stdlib.h>

int main(int argc , char *argv[])
{
    int vs=argc-1, v[vs], x;
    for (x=0; x < vs; x++) v[x]=atoi(argv[x+1]);
    qs(v, v+vs-1);
    for (x=0; x < vs; x++) printf("%d\n",v[x]);
    return 0;
}
#endif
