#include "htab_private.h"

//A good rule of thumb is to keep the load factor at 75% or less (some will say 70%)
//to maintain (very close to) O(1) lookup. Assuming you have a good hash function.
#define AVG_LEN_MAX 1.5f

htab_pair_t* htab_lookup_add(htab_t* t, htab_key_t key)
{
    size_t ind = htab_hash_function(key) % t->arr_size;

    htab_pair_t* pair = NULL;
    for (htab_item* head = t->arr_ptr[ind]; head; head = head->next)
    {
        if (scmp(head->pair.key, key))
            pair = &head->pair;
    }
    if (pair)
    {
        pair->value++;
        return pair;
    }

    htab_item* head = t->arr_ptr[ind];
    htab_item* new = NULL;
   
    if (!head)
    {
        if (!item_const(&t->arr_ptr[ind], key, 1))
            return NULL;
        
        pair = &t->arr_ptr[ind]->pair;
        goto end;
    }

    if (!item_const(&new, key, 1))
        return NULL;

    htab_item* head_next = head->next;

    head->next = new;
    new->next = head_next;
    pair = &new->pair;

end:
    t->size++;
    if ( (t->size / t->arr_size) > AVG_LEN_MAX )
        htab_resize(t, t->arr_size * 2);
    
    return pair;
}