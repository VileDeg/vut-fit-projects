#include "htab_private.h"

//A good rule of thumb is to keep the load factor at 75% or less (some will say 70%)
//to maintain (very close to) O(1) lookup. Assuming you have a good hash function.
#define AVG_LEN_MIN 0.375f

bool htab_erase(htab_t* t, htab_key_t key)
{
    size_t ind = htab_hash_function(key) % t->arr_size;

    htab_item* head = t->arr_ptr[ind];
    htab_item* prev = NULL;

    while (head && !scmp(head->pair.key, key))
    {
        if (!head->next)
            return false;
        
        prev = head;
        head = head->next;
    }

    nilfree(head->pair.key);
    if (!prev)
    {   
        htab_item* tofree = head;
        t->arr_ptr[ind] = head->next;
        nilfree(tofree);
    }
    else
    {
        prev->next = head->next;
        nilfree(head);
    }
    
    t->size--;
    if ( (t->size / t->arr_size) < AVG_LEN_MIN )
        htab_resize(t, t->arr_size / 2);
    
    return true;
}