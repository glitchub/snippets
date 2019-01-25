#include <stdio.h>
#include <stdlib.h>

// optimized bubble sort
int bs(int *a, int n)
{
    int i, l;
    while (l=n) for (i=n=0; i<l-1; i++) if (a[i] > a[i+1]) a[i] ^= (a[i+1] ^= (a[i] ^= a[i+1])), n=i+1;
}    

int main(int argc , char *argv[])
{
    int vs=argc-1, v[vs], x;
    for (x=0; x < vs; x++) v[x]=atoi(argv[x+1]);
    bs(v, vs);
    for (x=0; x < vs; x++) printf("%d ",v[x]); printf("\n");
} 
