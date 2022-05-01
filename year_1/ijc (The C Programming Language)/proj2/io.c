#include "io.h"
#include <ctype.h>

int read_word(char *s, int max, FILE *f)
{
	int c = 0;
	char* end = s + max;
	char* buff = s;

	while ( (c = fgetc(f)) != EOF && !isspace(c) && buff < end )
		*buff++ = c;

	*buff = '\0';

	return (c != EOF || buff != end-max ? buff - s : EOF);
}