#include "bitset.h"
//Extern inline functions prototypes
//to ensure their functionality with turned off optimalisation '-O0'
#ifdef USE_INLINE

	extern inline void bitset_free();

	extern inline ul bitset_size(bitset_t arr);

	extern inline void bitset_setbit(bitset_t arr, bitset_index_t ind, ul val);

	extern inline bool bitset_getbit(bitset_t arr, ul ind);

#endif //USE_INLINE