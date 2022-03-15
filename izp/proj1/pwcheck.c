/**
 * @name Projekt 1 - Prace s textem
 * @author Vadim Goncearenco <xgonce00@stud.fit.vutbr.cz>
 */
#include <stdio.h>
#include <stdbool.h>
//***CONSTANTS AND TYPES DECLARATIONS***
#define MIN_ARGC 3
#define MAX_ARGC 4
#define MIN_LEVEL 1
#define MAX_LEVEL 4
#define MAX_LEN 100

typedef unsigned long long int ulli;
enum OUT_CODES {SUCCES, NO_ARGS, ARGS_MANY, INV_INT, FARG_OB, INV_STATS, INV_LEN};
typedef struct Stats
{
    int signs_num[256];
    int sign_count;
    int min_len;
    int pw_count;
    double len_sum;
    double avg_len;
    bool exist;
} Stats;

//***FUNCTION PROTOTYPES***
int error_handling(int argc, char * argv[]);
bool validate_passwords(int lv, int pr, Stats *st_ptr);

bool condition_one(char curr_pw[], int len);
bool condition_two(char curr_pw[], int len, ulli pr);
bool condition_three(char curr_pw[], int len, ulli pr);
bool condition_four(char curr_pw[], int len, ulli pr);

bool condition_test(char curr_pw[], int len, int lv, ulli pr);

void count_stats(char pw[], Stats *st_ptr);
void print_stats(Stats *st_ptr);

int len(char s[]);
bool s_compare(char * str1, char * str2);
bool is_lower(char c);
bool is_upper(char c);
bool is_digit(char c);
bool is_special(char c);
ulli ptoi(char * c);

//***MAIN FUNCTION START***
int main(int argc, char *argv[])
{
    int lv = 0;
    ulli pr = 0;
    Stats st = {.signs_num = {-1}, .sign_count = 0, .min_len = -1, .pw_count = 0, .len_sum = 0.0f, .avg_len = 0.0f, .exist = false};
    Stats *st_ptr = &st;

    int ERR_CODE = error_handling(argc, argv);
	if (ERR_CODE) return ERR_CODE;

    lv = *argv[1] - '0';
    pr = ptoi(argv[2]);

    if (argc > 3) st_ptr->exist = true;
    
    if (!validate_passwords(lv, pr, st_ptr)) return INV_LEN;

	if (st.exist) print_stats(st_ptr);

    return SUCCES;
}
//***MAIN FUNCTION END***

//***FUNCTION IMPLEMENTATIONS***

//Returns non-zero value if the program was executed with wrong arguments
int error_handling(int argc, char *argv[])
{
	if (argc < MIN_ARGC)
    {
        fprintf(stderr, "\nERROR: Required arguments are not present!\n");
        return NO_ARGS;
    }
	if (argc > MAX_ARGC)
    {
        fprintf(stderr, "\nERROR: Too many arguments!\n");
        return ARGS_MANY;
    }
	if (!ptoi(argv[1]) || !ptoi(argv[2]))
	{
		fprintf(stderr, "\nERROR: Invalid integer arguments!\n");
        return INV_INT;
	}
	if ( (ptoi(argv[1]) < MIN_LEVEL) || (ptoi(argv[1]) > MAX_LEVEL) )
	{
		fprintf(stderr, "\nERROR: First (LEVEL) argument is out of bounds!\n");
        return FARG_OB;
	}
	if ( (argc > 3) && ( !s_compare(argv[3], "--stats") ))
	{
		fprintf(stderr, "\nERROR: Third argument must be \"--stats\"\n");
        return INV_STATS;
	}
	
	return 0;
}
//Finds and prints out valid passwords
//Returns 0 in case of invalid password length
bool validate_passwords(int lv, int pr, Stats *st_ptr)
{
    char pw[MAX_LEN + 1];
    while (fgets(pw, MAX_LEN + 2, stdin))
    {
        int l = len(pw);

        l = len(pw);

        if (l > MAX_LEN)
        {
            fprintf(stderr, "\nERROR: One or more passwords have invalid length!\n");
            return 0;
        }

        if (st_ptr->exist == 1) count_stats(pw, st_ptr);
        
        if (l < 1) continue;

        if ( condition_test(pw, l, lv, pr) )
        {
            printf("%s", pw);
        }
    }

    return 1;
}
//Prints out current session statistics
void print_stats(Stats *st_ptr)
{
    if (st_ptr->min_len == -1) st_ptr->min_len = 0;
    printf("Statistika:\n");
    printf("Ruznych znaku: %d\n", st_ptr->sign_count);
    printf("Minimalni delka: %d\n", st_ptr->min_len);
    printf("Prumerna delka: %0.1f\n", st_ptr->avg_len);
}
//Counts statistics of the current session
void count_stats(char pw[], Stats *st_ptr)
{
    st_ptr->pw_count++;
	
    int l = len(pw);
    st_ptr->len_sum += l;
    st_ptr->avg_len = st_ptr->len_sum / (double)st_ptr->pw_count;
    if ( (l < st_ptr->min_len) || (st_ptr->min_len == -1)) st_ptr->min_len = l;
	
    for (int i = 0; i < l; ++i)
    {
		if ( pw[i] != st_ptr->signs_num[ (int)pw[i] ] )
		{
			st_ptr->signs_num[ (int)pw[i] ] = pw[i];
			st_ptr->sign_count++;
		}
    }
}
//Returns 1 if the password matches all the conditions
bool condition_test(char curr_pw[], int len, int lv, ulli pr)
{
    if (lv >= 1)
    {
        if (!condition_one(curr_pw, len))
            return 0;
        if (lv >= 2)
        {
            if (!condition_two(curr_pw, len, pr))
                return 0;
            if (lv >= 3)
            {
                if (!condition_three(curr_pw, len, pr))
                    return 0;
                if (lv == 4)
                {
                    if (!condition_four(curr_pw, len, pr))
                        return 0;
                }
            }
        }
    }

    return 1;
}
//Returns 1 if the password matches the first condition
bool condition_one(char curr_pw[], int len)
{
    int l_count = 0, u_count = 0;
    for (int i = 0; i < len; i++)
    {
        if (is_lower(curr_pw[i]))
            l_count++;
        else if (is_upper(curr_pw[i]))
            u_count++;
    }
    if (l_count && u_count)
        return 1;
    else
        return 0;
}
//Returns 1 if the password matches the second condition
bool condition_two(char curr_pw[], int len, ulli pr)
{
    int l = 0, u = 0, d = 0, s = 0;
    for (int i = 0; i < len; i++)
    {
        if (is_lower(curr_pw[i])) l++;
        else if (is_upper(curr_pw[i])) u++;
        else if (is_digit(curr_pw[i])) d++;
        else if (is_special(curr_pw[i])) s++;
    }
    if (
        ((l && u && d && s) ) ||
            ( pr <= 3 && ( (l && u && d) || (l && u && s) || (l && d && s) || (u && d && s) ) ) ||
                ( pr <= 2 && ( (l && u) || (l && d) || (l && s) || (u && d) || (u && s) || (d && s) ) ) ||
                    ( pr == 1 && ( l || u || d || s ) )
        )
        return 1;
    else
        return 0;
}
//Returns 1 if the password matches the third condition
bool condition_three(char curr_pw[], int len, ulli pr)
{
    unsigned int count = 1;
	if (pr == 1) return 0;
    for (int i = 1; i < len; i++)
    {
        char last_c = curr_pw[i - 1];
        char curr_c = curr_pw[i];
		if (last_c == curr_c)
		{
			count++;
			if (count == pr)
				return 0;
		}
		else
			count = 1;
    }

    return 1;
}
//Returns 1 if the password matches the forth condition
bool condition_four(char curr_pw[], int len, ulli pr)
{
    int ss_c = len + 1 - pr;
    char sub_s[ss_c][pr + 1];

    for (int i = 0; i < ss_c; i++)
    {
        for (unsigned int j = 0; j < pr; j++)
        {
            sub_s[i][j] = curr_pw[i + j];
        }
        sub_s[i][pr] = '\0';
    }

    for (int i = 0; i < ss_c; i++)
    {
        for (int j = 0; j < ss_c; j++)
        {
            if (i == j) break;
            if (s_compare(sub_s[i], sub_s[j]))
            {
                return 0;
            }
        }
    }

    return 1;
}

//***HELPER FUNCTIONS***

//Returns 1 if two strings are equal
bool s_compare(char * s1, char * s2)
{
    int l1 = len(s1), l2 = len(s2);
	if (l1 != l2) return 0;
	for (int i = 0; i < l1; i++)
		if (s1[i] != s2[i]) return 0;

	return 1;
}
//Returns length of a string without '\0' and '\n'
int len(char s[])
{
    int count = 0;
    while (*s)
    {
        if (*s != '\n') count++;
        s++;
    }
        
    return count;
}
//Returns 1 if the character is lowercase
bool is_lower(char c)
{
    return (c >= 'a' && c <= 'z');
}
//Returns 1 if the character is uppercase
bool is_upper(char c)
{
    return (c >= 'A' && c <= 'Z');
}
//Returns 1 if the character is a digit number
bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}
//Returns 1 if the character is a special symbol from range bentween ' ' until '~'
bool is_special(char c)
{
    return ( (c >= ' ' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '\'') || (c >= '{' && c <= '~') );
}
//Converts a pointer to character to "unsigned long long int" type
ulli ptoi(char * c)
{
	long long int value = 0;

	for (int i = 0; i < len(c); i++)
	{
		if ((c[i] < '0' || c[i] > '9'))
		{
			value = -1;
			break;
		}
		
		value *= 10;
		
		if (i != 0)
			value += (c[i] - '0') * 10 * i;
		else
			value += c[i] - '0';
	}
	
	if (value == -1)
		return 0;
	
	return (ulli)value;
}