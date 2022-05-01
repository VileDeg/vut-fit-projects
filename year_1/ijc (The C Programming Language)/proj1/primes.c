// primes.c
// Řešení IJC-DU1, příklad a), 20.3.2022
// Autor: Vadim Goncearenco, FIT
// Přeloženo: gcc 7.5.0

#include <stdio.h>
#include <time.h>

#include "bitset.h"
#include "eratosthenes.h"

int main(void)
{
    double start = clock();
    
    //For bitset_create increase ulimit -s!
    //bitset_create(bs, 300000000ul);
    bitset_alloc(bs, 300000000ul);
    
    ul len = bitset_size(bs);

    Eratosthenes(bs);

    ul i = len-1;
    ul cnt = 0;
    //Move by 10 prime numbers back from the end
    for (; i > 0 && cnt < 10; i--)
    {
        if (!bitset_getbit(bs, i))
            cnt++;
        if (cnt == 10)
            break;
    }
    //Print out the 10 biggest prime numbers
    for (; i < len; i++)
    {
        if (!bitset_getbit(bs, i))
            printf("%lu\n", i);
    }

    bitset_free(bs);
    //Print out program execution time
    fprintf(stderr, "Time=%.3g\n", (double)(clock()-start)/CLOCKS_PER_SEC);
    return 0;
}