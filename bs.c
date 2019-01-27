// Optimized bubble sort. Useful for small data sets or in embedded context
// where stack size is limited. Otherwise use quick sort!

// Given array of n ints, sort the array in place
void bs(int *a, int n)
{
    int i, l;
    while (n) for (l=n, n=0, i=1; i<l; i++) if (a[i] < a[i-1]) a[i]^=a[i-1], a[i-1]^=a[i], a[i]^=a[i-1], n=i;
}

#if 1
// POC, accept integers on command line and print sorted list, e.g.:
//   ./bs $(for x in {1..10000}; do echo $RANDOM; done)
#include <stdio.h>
#include <stdlib.h>

int main(int argc , char *argv[])
{
    int vs=argc-1, v[vs], x;
    for (x=0; x < vs; x++) v[x]=atoi(argv[x+1]);
    bs(v, vs);
    for (x=0; x < vs; x++) printf("%d\n",v[x]);
    return 0;
}
#endif
