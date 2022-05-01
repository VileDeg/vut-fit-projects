#include "htab_private.h"

inline size_t htab_size(const htab_t* t) { return t->size; }
inline size_t htab_bucket_count(const htab_t* t) { return t->arr_size; }