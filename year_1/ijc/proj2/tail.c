// tail.c
// Řešení IJC-DU2, příklad a), 4.10.2022
// Autor: Vadim Goncearenco, FIT
// Přeloženo: gcc 7.5.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEM_ERR (-1)

#define errpr(x) do{ fprintf(stderr, "Error: " x "\n"); }while(0);
#define wrnpr(x) do{ fprintf(stderr, "Warning: " x "\n"); }while(0);
#define FREE(x) do{ free(x); x = NULL; }while(0);

#define MAX_LEN 4095

enum err
{
	SUCCES, ARG_MANY, NO_NUMBER, INV_NUMBER, INV_FILE
};

typedef struct
{
	size_t cap;
	size_t cnt;
	char** buff;
} cb_t;

void push_back(cb_t* cb, char* line)
{
	size_t last = cb->cap-1;

	for (size_t i = 0; i < last; i++)
	{
		strncpy(cb->buff[i], cb->buff[i+1], MAX_LEN);
	}
	
	size_t len = strlen(line);
	
	if (len > MAX_LEN)
	{
		wrnpr("Line length is bigger than MAX_LEN.");
	}

	strncpy(cb->buff[last], line, MAX_LEN);

	cb->cnt++;
}

int cb_const(cb_t* cb, size_t num)
{
	cb->cap = num;
	cb->cnt = 0;
	cb->buff = calloc(sizeof(char*), cb->cap);
	if (!cb->buff)
	{
		errpr("Memory allocation failed. Exiting program.");
		return MEM_ERR;
	}
		
	
	for (size_t i = 0; i < cb->cap; i++)
	{
		cb->buff[i] = calloc(1, MAX_LEN+1);
		if (!cb->buff[i])
		{
			FREE(cb->buff);
			errpr("Memory allocation failed. Exiting program.");
			return MEM_ERR;
		}
	}

	return 0;
}

void cb_dest(cb_t* cb)
{
	for (size_t i = 0; i < cb->cap; i++)
	{
		FREE(cb->buff[i]);
	}
	FREE(cb->buff);
}

void init(int argc, char** argv, long* num, FILE** fp)
{
	if (argc > 4)
	{
		errpr("Too many agruments.");
		exit(ARG_MANY);
	}

	bool narg = false;
	for (int i = 1; i < argc; i++)
	{
		char* a = argv[i];
		
		if (!strcmp(a, "-n"))
		{
			narg = true;
			
			if (i == argc-1)
			{
				errpr("No argument after '-n'.");
				exit(NO_NUMBER);
			}
		}
		else if (narg)
		{
			narg = false;

			char* endptr = NULL;
			
			*num = strtol(a, &endptr, 10);
			
			if (endptr[0] != '\0')
			{
				errpr("Invalid agrument after '-n'.");
				exit(INV_NUMBER);
			}
		}
		else if (a[0] != '<')
		{
			narg = false;
			
			*fp = fopen(a, "r");
			if (!fp)
			{
				errpr("Invalid file argument.");
				exit(INV_FILE);
			}
		}
	}
}

void read_file(cb_t* cb, FILE* fp, char* buff, size_t cap)
{
	while (fgets(buff, cap, fp))
	{
		push_back(cb, buff);
		memset(buff, 0, cap);
    }
}

void tail_print(cb_t cb)
{
	for (size_t i = 0; i < cb.cap; i++)
	{
		if (cb.buff[i][0] == '\0')
			continue;
		printf("%s", cb.buff[i]);
	}
}

int main(int argc, char** argv)
{
	long num = 10;
	FILE* fp = NULL;

	init(argc, argv, &num, &fp);
	
	fp = fp ? fp : stdin;

	int err = 0;
	cb_t cb = {num, 0, NULL};
	if ( (err = cb_const(&cb, num)) )
		return err;

	size_t cap = MAX_LEN+1;
	char* buff = calloc(1, cap);
	if (!buff)
	{
		errpr("Memory allocation failed. Exiting program.");
		return MEM_ERR;
	}

	read_file(&cb, fp, buff, cap);
	FREE(buff);

	tail_print(cb);

	cb_dest(&cb);
	if (fp) fclose(fp);

	return 0;
}