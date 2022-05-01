#include "htab_private.h"

void htab_for_each(const htab_t* t, void (*f)(htab_pair_t* data))
{
    for (size_t i = 0; i < t->arr_size; i++)
    {
        for (htab_item* head = t->arr_ptr[i]; head->pair.key; head = head->next)
            f(&head->pair);
    }
}