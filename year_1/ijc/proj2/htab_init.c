#include "htab_private.h"

htab_t* htab_init(size_t n)
{
    htab_t* ht = calloc(1, sizeof(htab_t));
    if (!ht)
    {
        perr("Memory allocation failed.");
        return NULL;
    }

    ht->size = 0;
    ht->arr_size = n;
    ht->arr_ptr = calloc(1, sizeof(htab_item*) * n);
    if (!ht->arr_ptr)
    {
        nilfree(ht);
        perr("Memory allocation failed.");
        return NULL;
    }
    
    return ht;
}
