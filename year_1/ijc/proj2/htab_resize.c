#include "htab_private.h"

void htab_resize(htab_t* t, size_t newn)
{
    htab_t* t_n = htab_init(newn);
    if (!t_n)
        return;
    
    for (size_t ind = 0; ind < t->arr_size; ind++)
    {
        for (htab_item* head = t->arr_ptr[ind]; head; head = head->next)
        {
            const char* old_key = head->pair.key;
            size_t old_val = head->pair.value;
            
            size_t ind_n = htab_hash_function(old_key) % t_n->arr_size;
            
            htab_item* head_n = t_n->arr_ptr[ind_n];
            htab_item* new = NULL;
        
            if (!head_n)
            {
                if (!item_const(&t_n->arr_ptr[ind_n], old_key, old_val))
                {
                    htab_free(t_n);
                    return;
                }   

                t_n->size+=old_val;
                continue;
            }

            if (!item_const(&new, old_key, old_val))
            {
                htab_free(t_n);
                return;
            }

            htab_item* head_n_next = head_n->next;

            head_n->next = new;
            new->next = head_n_next;

            t_n->size+=old_val;
        }
        
    }

    htab_clear(t);
    nilfree(t->arr_ptr);
    t->arr_ptr = t_n->arr_ptr;
    t->arr_size = t_n->arr_size;
    t->size = t_n->size;

    nilfree(t_n);
}