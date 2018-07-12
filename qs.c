#include <stdio.h>
#include <stdlib.h>

static void _qs(int *a, int lo, int hi) 
{
    if (hi > lo)
    {
        int l=lo, h=hi, p=a[(hi+lo)/2];
        while (1)
        {
            int t;
            while (a[l] < p) l++;
            while (a[h] > p) h--;
            if (l >= h) break;
            t=a[l]; a[l++]=a[h]; a[h--]=t;
        }
        _qs(a, lo, h); 
        _qs(a, h+1, hi);  
    }   
}
#define qs(array, size) _qs(array, 0, size-1)

int main(int argc, char *argv[]) 
{
   int x;
   int size=argc>1?atoi(argv[1]):10;
   int array[size];

   for (x = 0; x < size; x++) array[x]=random()&31;
   for (x = 0; x < size; x++) printf("%d ",array[x]); printf("\n\n");
   qs(array, size);
   for (x = 0; x < size; x++) printf("%d ",array[x]); printf("\n");
   return 0;
}
