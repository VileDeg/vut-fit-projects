// wordcount.c
// Řešení IJC-DU2, příklad b), 4.10.2022
// Autor: Vadim Goncearenco, FIT
// Přeloženo: gcc 7.5.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htab.h"
#include "io.h"
//Should be a prime number.
//11 is default hashtable size in Java.
#define HTAB_INI_SIZE 11

int main(void)
{
	htab_t* t = htab_init(HTAB_INI_SIZE);
	if (!t)
		return -1;
	
	char* word = calloc(1, MAX_KEY_LEN+2);
	if (!word)
	{
		fprintf(stderr, "Memory allocation failed.\n");
		htab_free(t);
		return -1;
	}
	int send_warning = true;
	
	while (read_word(word, MAX_KEY_LEN+1, stdin) != EOF)
	{
		if (send_warning && (word[MAX_KEY_LEN] != '\0'))
		{
			fprintf(stderr, "Warning: word of length bigger than %d\n", MAX_KEY_LEN);
			send_warning = false;
		}
		word[MAX_KEY_LEN] = '\0';
		
		if (!htab_lookup_add(t, word))
			return -1;
	}
	free(word);
	
	htab_print(t);
	
	htab_free(t);
	return 0;
}