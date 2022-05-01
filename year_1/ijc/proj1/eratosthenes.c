#include "eratosthenes.h"

void Eratosthenes(bitset_t bs)
{
	ul len = bitset_size(bs);
    //0 and 1 are not prime numbers
    bitset_setbit(bs, 0, 1);
    bitset_setbit(bs, 1, 1);
    //Null-out the rest of the array
    for (ul i = 2; i < len; i++)
    {
        bitset_setbit(bs, i, 0);
    }
    //Execute algorithm
    for (ul i = 2; i < sqrt(len); i++)
    {
        if (bitset_getbit(bs, i) == 0)
        {
            for (ul j = i+i; j < len; j+=i)
                bitset_setbit(bs, j, 1);
        }
    }
}