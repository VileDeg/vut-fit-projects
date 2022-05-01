#include "htab_private.h"

htab_pair_t* htab_find(htab_t* t, htab_key_t key)
{
    size_t ind = htab_hash_function(key) % t->arr_size;

    for (htab_item* head = t->arr_ptr[ind]; head; head = head->next)
    {
        if (scmp(head->pair.key, key))
            return &head->pair;
    }
    
    return NULL;
}