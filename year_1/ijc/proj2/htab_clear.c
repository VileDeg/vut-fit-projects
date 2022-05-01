#include "htab_private.h"

void htab_clear(htab_t* t)
{
    for (size_t i = 0; i < t->arr_size; i++)
    {       
        for (htab_item* head = t->arr_ptr[i]; head; )
        {
            nilfree(head->pair.key);
            htab_item* next = head->next;
            nilfree(head);
            head = next;
        }   
    }

    t->size = 0;
}