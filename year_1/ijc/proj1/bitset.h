#ifndef _BITSET_H_
#define _BITSET_H_

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "error.h"

#define MEM_ERR (-1) //Error code for memory allocation error

#define UL_SIZE sizeof(bitset_index_t) //Size of unsigned long in bytes
#define UL_BITSIZE (UL_SIZE*8u) //Size of unsigned long in bits

#define MIN_LEN (1ul*UL_SIZE) //Minimum allowed length for a bit array
#define MAX_LEN (ULONG_MAX) //Maximum allowed length for a bit array

typedef unsigned u;
typedef unsigned long ul;
typedef unsigned long bitset_index_t; //Type of elements in bit array

typedef bitset_index_t* bitset_t; //Bit array type
//By default macros will be used to operate with bit array
#ifndef USE_INLINE 
    //Initialized bit array on stack
    #define bitset_create(arr, len) \
        _Static_assert(((ul)len >= MIN_LEN) && ((ul)len < MAX_LEN), "Invalid length.\n"); \
        bitset_index_t arr[ (ul)len/UL_SIZE + (((ul)len % UL_SIZE) != 0) +1 ] = {(ul)len,0,};
    //Allocates memory for bit array on heap
    #define bitset_alloc(arr, len) \
        assert(((ul)len >= MIN_LEN) && ((ul)len < MAX_LEN)); \
        bitset_t arr = calloc( UL_SIZE, (ul)len/UL_SIZE + (((ul)len % UL_SIZE) != 0) +1 ); \
        if (!arr) \
        { \
            fprintf(stderr, "bitset_alloc: Chyba alokace paměti.\n"); \
            exit(MEM_ERR); \
        } \
        arr[0] = (ul)len; \
    //Frees memory allocated for bit array
    #define bitset_free(arr) do { \
        free((void*)arr); \
        arr = NULL; \
    } while(0)
    //Returnes amount of bits stored in the array
    #define bitset_size(arr) ( ((bitset_t)arr)[0] )
    //Sets the bit with index 'ind' to 0 or 1 according to 'val'
    #define bitset_setbit(arr, ind, val) do { \
        ul _size = bitset_size(arr); \
        if ((ul)ind >= _size) \
            error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu.\n", (ul)ind, _size); \
        ul _ind = (ul)ind+UL_BITSIZE; \
        if (val) \
            ((bitset_t)arr)[(_ind)/UL_BITSIZE] |= ( 1ul << ((_ind) % UL_BITSIZE) ); \
        else \
            ((bitset_t)arr)[(_ind)/UL_BITSIZE] &= ( ~(1ul << ((_ind) % UL_BITSIZE)) ); \
    } while(0)
    //Returnes value of the bit with index 'ind'
    #define bitset_getbit(arr, ind) ( \
        ((ul)ind >= bitset_size(arr)) ? error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu.\n",\
            (ul)ind, bitset_size(arr)), (bool)0 : (bool)(\
            ( \
                ((bitset_t)arr)[((ul)ind + UL_BITSIZE) / UL_BITSIZE] & \
                ( 1ul << (((ul)ind + UL_BITSIZE) % UL_BITSIZE) ) \
            ) \
            >> (((ul)ind + UL_BITSIZE) % UL_BITSIZE) ) \
    )
//If the program is compile with '-DUSE_INLINE' flag, macros will be replaced with inline function where possible
#else //USE_INLINE -------------------------------------------------------------------------------

    #define bitset_create(arr, len) \
        _Static_assert(((ul)len >= MIN_LEN) && ((ul)len < MAX_LEN), "Invalid length.\n"); \
        bitset_index_t arr[ (ul)len/UL_SIZE + (((ul)len % UL_SIZE) != 0) +1 ] = {(ul)len,0,};

    #define bitset_alloc(arr, len) \
        assert(((ul)len >= MIN_LEN) && ((ul)len < MAX_LEN)); \
        bitset_t arr = calloc( UL_SIZE, (ul)len/UL_SIZE + (((ul)len % UL_SIZE) != 0) +1 ); \
        if (!arr) \
        { \
            fprintf(stderr, "bitset_alloc: Chyba alokace paměti.\n"); \
            exit(MEM_ERR); \
        } \
        arr[0] = (ul)len; \

    
    inline void bitset_free(bitset_t arr)
    {
        free(arr);
        arr = NULL;
    }

    inline ul bitset_size(bitset_t arr) { return arr[0]; }

    inline void bitset_setbit(bitset_t arr, bitset_index_t ind, ul val)
    {
        ul size = bitset_size(arr);
        if (ind >= size)
            error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu.\n", ind, size);
        
        ind += UL_BITSIZE;
        if (val)
            arr[(ind)/UL_BITSIZE] |= ( 1ul << ((ind) % UL_BITSIZE) );
        else
            arr[(ind)/UL_BITSIZE] &= ( ~(1ul << ((ind) % UL_BITSIZE)) );
    }

    inline bool bitset_getbit(bitset_t arr, bitset_index_t ind)
    {
        ul size = bitset_size(arr);
        if (ind >= size)
            error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu.\n", ind, size);

        ind += UL_BITSIZE;
        ul byte_num = ind / UL_BITSIZE;
        ul offset = ind % UL_BITSIZE;

        return ( (arr[byte_num] & (1ul << offset)) >> offset );
    }

#endif //USE_INLINE

#endif //_BITSET_H_
