#ifndef __HTAB_PRIVATE_H__
#define __HTAB_PRIVATE_H__

#include <stdio.h>
#include <stdlib.h>

#include "htab.h"

#define scmp(x,y) ( (x) && (y) ? !strcmp(x,y) : false )
#define scpy(dst, src) do{ strncpy((char*)dst, src, MAX_KEY_LEN); }while(0);
#define nilfree(x) do{ free((void*)(x)); (x) = NULL; }while(0);
#define perr(x) do{ fprintf(stderr, "Error \"" x "\" in \"%s\" on line: %d\n", __func__, __LINE__); }while(0);

typedef struct htab_item
{
    htab_pair_t pair;
    struct htab_item* next;
} htab_item;

struct htab
{
    size_t size;
    size_t arr_size;
    htab_item** arr_ptr;
};

bool key_const(htab_key_t* dst, htab_key_t src);
htab_pair_t* item_const(htab_item** item, htab_key_t src_key, htab_value_t src_val);

#endif  // __HTAB_PRIVATE_H__