#include "htab_private.h"

void pair_print(htab_pair_t p) { printf("[(%-5s, %d) ", p.key, p.value); }

void item_print(htab_item* it)
{
    pair_print(it->pair);
    printf("-> %d] |--> ", (bool)it->next);
}

void htab_print_debug(htab_t* t)
{
    printf("%lu\n", sizeof(htab_t));
    printf("%lu\n", sizeof(htab_item));
    for (size_t i = 0; i < t->arr_size; i++)
    {
        printf("%lu: ", i);
        

        for (htab_item* head = t->arr_ptr[i]; head; head = head->next)
            item_print(head);   
        
        printf("\n-----------------------------\n");
    }
}

void htab_print(htab_t* t)
{
	for (size_t i = 0; i < t->arr_size; i++)
    {
        for (htab_item* head = t->arr_ptr[i]; head; head = head->next)
        {
            htab_pair_t p = head->pair;
            printf("%s\t%d\n", p.key, p.value);
        }
    }
}

bool key_const(htab_key_t* dst, htab_key_t src)
{
    if (!src)
    {
        *dst = NULL;
        return true;
    }

    *dst = calloc(1, MAX_KEY_LEN+1);
    if (!(*dst))
    {
        perr("Memory allocation failed.");
        return false;
    }
    
    scpy(*dst, src);
    return true;
}

htab_pair_t* item_const(htab_item** item, htab_key_t src_key, htab_value_t src_val)
{
    *item = calloc(1, sizeof(htab_item));
    if (!item)
    {
        perr("Memory allocation failed.");
        return NULL;
    }

    if (!key_const(&(*item)->pair.key, src_key))
    {
        nilfree(*item);
        return NULL;
    }

    (*item)->pair.value = src_val;
    (*item)->next = NULL;

    return &(*item)->pair;
}
