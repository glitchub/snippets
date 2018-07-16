#include <stdio.h>
#include <stdlib.h>

static void qs(int *a, int lo, int hi) 
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
        qs(a, lo, h); 
        qs(a, h+1, hi);  
    }   
}

int main(int argc, char *argv[]) 
{
   int x, size=argc-1, array[size];
   for (x = 0; x < size; x++) array[x]=atoi(argv[x+1]);
   qs(array, 0, size-1);
   for (x = 0; x < size; x++) printf("%d ",array[x]); 
   printf("\n");
   return 0;
}
