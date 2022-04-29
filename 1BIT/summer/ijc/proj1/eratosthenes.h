#ifndef _ERATOSTHENES_H_
#define _ERATOSTHENES_H_

#include <math.h>

#include "bitset.h"

//Sieve of Eratosthenes(https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)
//Sets every bit that has a "prime number" index to 0
void Eratosthenes(bitset_t bs);

#endif //_ERATOSTHENES_H_