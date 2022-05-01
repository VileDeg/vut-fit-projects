/**
 * @name Projekt 2 - Prace s datovymi strukturami
 * @author Vadim Goncearenco <xgonce00@stud.fit.vutbr.cz>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
//---Macros
#define MAX_ELEM_LEN 30 //Maximum allowed element length
#define MAX_LINE_COUNT 1000 //Maximum allowed line count
#define M_FTOR 2 //Memory factor constant. Used to determine capacity resize rate
#define FREE(x) do { if (x) {free(x); x = NULL;} } while(0);
//---Enums
typedef enum //Enumerable type representing return values of several function
{
    SUCCESS, MEM_ERR, INV_ARGS, NO_FILE, UNI_NOTFIRST, INV_ELEM, INV_UNI, INV_LETTER, NO_OPER, INV_OPER, LINES_MANY, LINE_EMPTY,
    OPER_FEW, OPER_MANY, OP_ARGS_BAD, OPER_MISS, ELEM_DUP, INV_SEQUENCE, NO_SETREL, NO_COMMAND
} ERRCODE;
typedef enum //Enumerable type representing operating mode of "s_equality" function
{
    SUBSET, SUBSETEQ, EQUAL
} EQUALITY;

//---Helper functions
char* strdup(const char* src) //Returnes a copy of argument string on heap
{
    size_t len = strlen(src) + 1;
    char *s = malloc(len);
    if (!s)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }
    return memcpy(s, src, len);
}

void rmv_chars(char* s, char c1, char c2) //Removes characters "c1", "c2" from string "s"
{
    int i, j;
    int len = strlen(s);
    for(i = 0; i < len; i++)
    {
        if( s[i] == c1 || s[i] == c2)
        {
            for(j = i; j < len; j++)
            {
                s[j] = s[j + 1];
            }

            len--;
            i--;
        }
    }
    s[len] = '\0';
}

//---Structs and their "methods"
typedef struct
{
    bool set_rel_found; //Must be true if a set or relation was found in file
    bool command_found; //Must be true if a command was found in file
} Condition;

ERRCODE cond_check(Condition cond)
{
    if (!cond.set_rel_found)
    {
        fprintf(stderr, "Error: [%s] [%d] no set/relation found in file.", __func__, NO_SETREL);
        return NO_SETREL;
    }
    
    if (!cond.command_found)
    {
        fprintf(stderr, "Error: [%s] [%d] no command found in file.", __func__, NO_COMMAND);
        return NO_COMMAND;
    }

    return SUCCESS;
}

typedef struct //A "dynamic" string type structure. Allocated on heap
{
    size_t b_cap; //Buffer capacity. Should not be changed
    size_t b_used; //Buffer "real" length
    char* buff; //Buffer. A statically sized temporary container string
    size_t s_cap; //"Actual" string capacity. Increased when needed.
    size_t s_used; //"Actual" string "real" length.
    char* str; //"Actual" string. A dynamically sized container string
} DynaS;

DynaS* ds_const()
{
    DynaS* ds = malloc(sizeof(*ds));
    if (!ds)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }
    ds->b_cap = 128;
    ds->b_used = 0;
    ds->buff = calloc(ds->b_cap, 1);
    if (!ds->buff)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        FREE(ds);
        return NULL;
    }

    ds->s_cap = 128;
    ds->s_used = 0;
    ds->str = calloc(ds->s_cap, 1);
    if (!ds->str)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        FREE(ds->buff);
        FREE(ds);
        return NULL;
    }

    return ds;
}
void ds_dest(DynaS* ds)
{
    FREE(ds->buff);
    FREE(ds->str);
    FREE(ds);
}
void ds_bw(DynaS* ds, char* s) //Dynamic_string_buffer_write : writes "s" to buffer of "ds" and sets it's calculated it's actual length
{
    ds->buff = s;
    ds->b_used = strlen(ds->buff);
}
bool ds_append(DynaS* ds) //Appends contents of buffer to the end of actual string, doubling it's capacity if needed.
{
    ds->b_used = strlen(ds->buff);

    if (ds->b_used > ds->s_cap - ds->s_used)
    {

        char* tmp = realloc(ds->str, ds->s_cap *= 2);
        if (!tmp)
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            ds_dest(ds);
            return false;
        }
        ds->str = tmp;
    }
    
    strcat(ds->str, ds->buff);
    ds->s_used += ds->b_used;

    ds->buff = memset(ds->buff, 0, ds->b_cap);
    ds->b_used = 0;

    return true;
}
void ds_clear(DynaS* ds) //Clears contents of buffer and actual string
{
    ds->str = memset(ds->str, 0, ds->s_cap);
    ds->s_used = 0;

    ds->buff = memset(ds->buff, 0, ds->b_cap);
    ds->b_used = 0;
}
void ds_rmnl(DynaS* ds) //Removes \r\n symbols from the end of actual string
{
    if (ds->s_used < 2)
        return;
    
    strtok(ds->str, "\r\n");
    ds->s_used = strlen(ds->str);
}

typedef struct //A set type structure. Allocated on heap
{
    size_t cap; //Dynamic capacity for storing elements
    size_t index; //Number of row in file
    size_t cnt; //Amount of elements
    char** elm; //Dynamically sized list of elements
} Set;

Set* set_const()
{
    Set* s = malloc(sizeof(*s));
    if (!s)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }
    s->cap = 1;
    s->index = 0;
    s->cnt = 0;
    s->elm = NULL;

    return s;
}
void set_dest(Set* s)
{
    for (size_t i = 0; i < s->cnt; i++)
        FREE(s->elm[i]);
    FREE(s->elm);
    FREE(s);
}
bool set_add(Set* s, char* elem) //Adds the element to the given set
{
    if ( s->cnt >= s->cap-1 ) //If capacity is exceeded, it is increased depending on M_FTOR(memory factor constant)
    {
        char** tmp = realloc( s->elm, sizeof(char*) * (s->cap *= M_FTOR) );
        if (!tmp)
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            FREE(s->elm);
            return false;
        }

        s->elm = tmp;
    }
    
    s->elm[s->cnt] = strdup(elem); //Element is copied on heap to ensure it's consistency
    if (!s->elm[s->cnt])
    {
        FREE(s->elm);
        return false;
    }

    s->cnt++;
    return true;
}
void set_print(Set* s) //Prints the set out in correct format
{
    printf("%s", "S");
    for (size_t i = 0; i < s->cnt; i++)
    {
        printf(" %s", s->elm[i]);
    }
    printf("\n");
}
Set* set_full(char* line, size_t line_index) //Returns a set filled with elements, that are extracted from "line"
{
    Set* s = set_const();
    if (!s) return NULL;

    char* l_copy = strdup(line); //Copies "line" to heap to prevent unintended behaviour
    if (!l_copy)
    {
        set_dest(s);
        return NULL;
    }
    //WARNING: strtok() modifies passed string!
    char* tk = strtok(l_copy, " ");
    for ( tk = strtok(NULL, " ");  tk != NULL; tk = strtok(NULL, " ") ) //Consequetively extract all tokens from line
        if (!set_add(s, tk))
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            FREE(l_copy);
            set_dest(s);
            return NULL;
        }
    
    s->index = line_index;

    FREE(l_copy);
    return s;
}

typedef struct //A relation type structure. Allocated on heap
{
    size_t cap; //Dynamic capacity for storing elements
    size_t index; //Number of row in file
    size_t cnt; //Amount of elements
    char** v1; //Dynamically sized list of elements' first value
    char** v2; //Dynamically sized list of elements' second value
    Set* s; //A set containing all elements' values in order
} Rel;

Rel* rel_const()
{
    Rel* r = malloc(sizeof(*r));
    if (!r)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }

    r->v1 = NULL;
    r->v2 = NULL;
    r->s = set_const();
    r->cap = 1;
    r->cnt = 0;
    r->index = 0;
    
    return r;
}
void rel_dest(Rel* r)
{
    for (size_t i = 0; i < r->cnt; i++)
    {
        FREE(r->v1[i]);
        FREE(r->v2[i]);
    }
    
    FREE(r->v1);
    FREE(r->v2);
    set_dest(r->s);
    FREE(r);
}
bool rel_add(Rel* r, char* v1, char* v2) //Adds the element to the given relation
{
    char** tmp = NULL;
    if (r->cnt >= r->cap-1) //If capacity is exceeded, it is increased depending on M_FTOR(memory factor constant)
    {
        tmp = realloc( r->v1, sizeof(char*) * (r->cap * M_FTOR) );
        if (!tmp)
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            FREE(r->v1);
            return false;
        }
        r->v1 = tmp;

        tmp = realloc( r->v2, sizeof(char*) * (r->cap * M_FTOR) );
        if (!tmp)
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            FREE(r->v1);
            FREE(r->v2);
            return false;
        }
        r->v2 = tmp;
        r->cap *= M_FTOR;
    }
   
    r->v1[r->cnt] = strdup(v1); //Element is copied on heap to ensure it's consistency
    if (!r->v1[r->cnt])
    {
        FREE(r->v1);
        FREE(r->v2);
        return false;
    }
    
    r->v2[r->cnt] = strdup(v2); //Element is copied on heap to ensure it's consistency
    if (!r->v2[r->cnt])
    {
        FREE(r->v1);
        FREE(r->v2);
        FREE(r->v1[r->cnt]);
        return false;
    }
  
    r->cnt++;

    if (!set_add(r->s, v1))
    {
        FREE(r->v1);
        FREE(r->v2);
        FREE(r->v1[r->cnt]);
        FREE(r->v2[r->cnt]);
        return false;
    }
    if (!set_add(r->s, v2))
    {
        FREE(r->v1);
        FREE(r->v2);
        FREE(r->v1[r->cnt]);
        FREE(r->v2[r->cnt]);
        return false;
    }

    return true;
}
void rel_print(Rel* r)
{
    printf("%s", "R");
    for (size_t i = 0; i < r->cnt; i++)
    {
        printf(" (%s ", r->v1[i]);
        printf("%s)", r->v2[i]);
    }
    printf("\n");
}
Rel* rel_full(char* line, size_t line_index) //Returns a relation filled with elements, that are extracted from "line"
{
    Rel* r = rel_const();
    if (!r) return NULL;

    char* l_copy = strdup(line);//Copies "line" to heap to prevent unintended behaviour
    if (!l_copy)
    {
        rel_dest(r);
        return NULL;
    } 
    
    //WARNING: strtok() modifies passed string!
    char* tk = strtok(l_copy, " ");

    char* v1 = NULL;
    char* v2 = NULL;
    int i = 0;
    for (tk = strtok(NULL, " ");  tk != NULL; tk = strtok(NULL, " ") ) //Consequetively extract all tokens from line
    {
        rmv_chars(tk, '(', ')');

        if (i % 2)
        {
            v2 = tk;
            if (!rel_add(r, v1, v2))
            {
                rel_dest(r);
                FREE(l_copy);
                return NULL;
            }
        }
        else
            v1 = tk;
        i++;
    }

    r->index = line_index;

    FREE(l_copy);
    return r;
}

typedef struct //A set list type structure. Allocated on heap
{
    size_t cap; //Dynamic capacity for storing sets
    size_t cnt; //Amount of sets
    Set** s; //Dynamically sized list of sets
} SetList;

SetList* setlist_const()
{
    SetList* sl = malloc( sizeof(*sl) );
    if (!sl)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }

    sl->s = NULL;
    sl->cap = 1;
    sl->cnt = 0;
    return sl;
}
bool setlist_add(SetList* sl, Set* s) //Adds the set to the given set list
{
    if (sl->cnt >= sl->cap-1) //If capacity is exceeded, it is increased depending on M_FTOR(memory factor constant)
    {
        Set** tmp = realloc(sl->s, sizeof(s) * (sl->cap *= M_FTOR));
        if (!tmp)
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            for (size_t i = 0; i < sl->cnt; i++)
                set_dest(sl->s[i]);
            
            return false;
        }
        sl->s = tmp;
    }

    sl->s[sl->cnt++] = s;
    return true;
}
void setlist_dest(SetList* sl)
{
    for (size_t i = 0; i < sl->cnt; i++)
        set_dest(sl->s[i]);
    FREE(sl->s);
    FREE(sl);
}
Set* get_set(SetList* sl, size_t index) //Returns the set with the specified index
{
    if (!index) return NULL;
    for (size_t i = 0; i < sl->cnt; i++)
    {
        if (sl->s[i]->index == index)
            return sl->s[i];
    }
    return NULL;
}

typedef struct //A relation list type structure. Allocated on heap
{
    size_t cap; //Dynamic capacity for storing relations
    size_t cnt; //Amount of relations
    Rel** r; //Dynamically sized list of relations
} RelList;

RelList* rellist_const()
{
    RelList* rl = malloc( sizeof(*rl) );
    if (!rl)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }

    rl->r = NULL;
    rl->cap = 1;
    rl->cnt = 0;
    return rl;
}
bool rellist_add(RelList* rl, Rel* r) //Adds the relation to the given set list
{
    if (rl->cnt >= rl->cap-1)
    {
        Rel** tmp = realloc(rl->r, sizeof(r) * (rl->cap *= M_FTOR));
        if (!tmp)
        {
            fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
            for (size_t i = 0; i < rl->cnt; i++)
                rel_dest(rl->r[i]);
            return false;
        }
        rl->r = tmp;
    }

    rl->r[rl->cnt++] = r;
    return true;
}
void rellist_dest(RelList* rl)
{
    for (size_t i = 0; i < rl->cnt; i++)
        rel_dest(rl->r[i]);
    FREE(rl->r);
    FREE(rl);
}
Rel* get_rel(RelList* rl, size_t index) //Returns the relation with the specified index
{
    for (size_t i = 0; i < rl->cnt; i++)
    {
        if (rl->r[i]->index == index)
            return rl->r[i];
    }
    return NULL;
}

typedef struct //A operation type structure
{
    SetList* sl; //List of all found sets
    RelList* rl; //List of all found relations
    Set* uni; //Univerzum set
    char* name;
    size_t index_1; //Operation argument 1. Must not be undefined
    size_t index_2; //Operation argument 2. Can be undefined
    size_t index_3; //Operation argument 3. Can be undefined
} Operation;

Operation* op_const()
{
    Operation* op = malloc( sizeof(*op) );
    if (!op)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        return NULL;
    }
    op->sl = setlist_const();
    if (!op->sl)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        FREE(op);
        return NULL;
    }
    op->rl = rellist_const();
    if (!op->rl)
    {
        fprintf(stderr, "Error [%s]: memory allocation failed!", __func__);
        FREE(op->sl);
        FREE(op);
        return NULL;
    }
    op->uni = NULL;
    op->name = NULL;
    op->index_1 = 0;
    op->index_2 = 0;
    op->index_3 = 0;

    return op;
}
void op_dest(Operation* op)
{
    setlist_dest(op->sl);
    rellist_dest(op->rl);
    FREE(op->name);
    FREE(op);
}

bool is_op_ternary(char * s) //Returnes "true" if argument is ternary operation name
{
    if (
        !strcmp(s, "injective") || !strcmp(s, "surjective") || 
        !strcmp(s, "bijective")
        )
        return true;
    else
        return false;
    
}
bool is_op_rel(char* s) //Returnes "true" if argument is relation operation name
{
    if (
        !strcmp(s, "reflexive")     || !strcmp(s, "symmetric")   || !strcmp(s, "antisymmetric") ||
        !strcmp(s, "transitive")    || !strcmp(s, "function")    || !strcmp(s, "domain")        ||
        !strcmp(s, "codomain")      || !strcmp(s, "closure_ref") || !strcmp(s, "closure_sym")   ||
        !strcmp(s, "closure_trans") ||
        is_op_ternary(s)
        )
        return true;
    else
        return false;
}
bool is_op_unary(char* s) //Returnes "true" if argument is unary operation name
{
    if (
        !strcmp(s, "empty")         || !strcmp(s, "card")        || !strcmp(s, "complement")    || 
        !strcmp(s, "reflexive")     || !strcmp(s, "symmetric")   || !strcmp(s, "antisymmetric") ||
        !strcmp(s, "transitive")    || !strcmp(s, "function")    || !strcmp(s, "domain")        ||
        !strcmp(s, "codomain")      || !strcmp(s, "closure_ref") || !strcmp(s, "closure_sym")   ||
        !strcmp(s, "closure_trans")
        )
        return true;
    else
        return false;
}
bool is_op_binary(char * s) //Returnes "true" if argument is binary operation name
{
    if (
        !strcmp(s, "union")      || !strcmp(s, "intersect") || !strcmp(s, "minus") || 
        !strcmp(s, "subseteq")   || !strcmp(s, "subset")    || !strcmp(s, "equals")
        )
        return true;
    else
        return false;
    
}
bool is_op_name(char* s) //Returnes "true" if argument is operation name
{
    if ( is_op_unary(s) || is_op_binary(s) || is_op_ternary(s))
        return true;
    else
        return false;
}

ERRCODE handle_errors(int argc) //Validates program execution arguments. The first function to be called
{
    if (argc != 2)
    {
        fprintf(stderr, "ERROR: invalid number of arguments!");
        return INV_ARGS;
    }

    return SUCCESS;
}
ERRCODE uni_check(char* token, Set* uni) //Checks if "token" is present in univerzum
{
    bool same = false;
    for (size_t i = 0; i < uni->cnt; i++)
        if ( !strcmp(token, uni->elm[i]) )
            same = true;

    if (!same)
    {
        fprintf(stderr, "Error [uni_check]: invalid element [] (isn't present in univerzum)");
        return INV_ELEM;
    }

    return SUCCESS;
}
ERRCODE comm_validate(Operation* op) //Validates command and sets value of passed variables
{
    //First token was extracted in process_line()
    //Following strtok calls are modifying previous string due to static reference!
    char* tk = strtok(NULL, " "); //Second token
    if (!tk)
    {
        fprintf(stderr, "Error: Operation name is missing.");
        return NO_OPER;
    }
    
    FREE(op->name);
    op->name = strdup(tk);
    if (!op->name)
        return MEM_ERR;

    if (is_op_unary(tk))
    {
        tk = strtok(NULL, " "); //Third token
        //Must be not NULL. Unary operator must have one argument!
        if (!tk)
        {
            fprintf(stderr, "Error: Operation has not enough arguments.\n");
            return OPER_FEW;
        }
        
        char* endptr = NULL;
        op->index_1 = strtol(tk, &endptr, 10); //Convert to digits
        //Must be a digit number!
        if (*endptr != '\0') //Invalid arguments
        {
            fprintf(stderr, "Error: Operation has invalid arguments.\n");
            return OP_ARGS_BAD;
        }
    }
    else if (is_op_binary(tk))
    {
        tk = strtok(NULL, " "); //Third token
        char* tk_two = strtok(NULL, " "); //Forth token
        //Both must not be NULL. Binary operator must have two arguments!
        if (!tk || !tk_two) 
        {
            fprintf(stderr, "Error: Operation has not enough arguments.");
            return OPER_FEW;
        }
        
        char* endptr = NULL;
        op->index_1 = strtol(tk, &endptr, 10);
        //Must be a digit number!
        if (*endptr != '\0')
        {
            fprintf(stderr, "Error: Operation has invalid arguments.");
            return OP_ARGS_BAD;
        }

        op->index_2 = strtol(tk_two, &endptr, 10);
         //Must be a digit number!
        if (*endptr != '\0')
        {
            fprintf(stderr, "Error: Operation has invalid arguments.");
            return OP_ARGS_BAD;
        }
    }
    else if (is_op_ternary(tk))
    {
        tk = strtok(NULL, " "); //Third token
        char* tk_two = strtok(NULL, " "); //Forth token
        char* tk_three = strtok(NULL, " "); //Fifth token
        //Both must not be NULL. Binary operator must have two arguments!
        if (!tk || !tk_two || !tk_three) 
        {
            fprintf(stderr, "Error: Operation has not enough arguments.");
            return OPER_FEW;
        }
        
        char* endptr = NULL;
        op->index_1 = strtol(tk, &endptr, 10);
        //Must be a digit number!
        if (*endptr != '\0')
        {
            fprintf(stderr, "Error: Operation has invalid arguments.");
            return OP_ARGS_BAD;
        }

        op->index_2 = strtol(tk_two, &endptr, 10);
        //Must be a digit number!
        if (*endptr != '\0')
        {
            fprintf(stderr, "Error: Operation has invalid arguments.");
            return OP_ARGS_BAD;
        }

        op->index_3 = strtol(tk_three, &endptr, 10);
        //Must be a digit number!
        if (*endptr != '\0')
        {
            fprintf(stderr, "Error: Operation has invalid arguments.");
            return OP_ARGS_BAD;
        }
    }
    else //If operator is not unary/binary/ternary, it is considered invalid
    {
        fprintf(stderr, "Error: Invalid operation name.");
        return INV_OPER;
    }

    tk = strtok(NULL, " "); //Get exceeding token
    //Must be NULL, otherwise too many arguments were passed
    if (tk)
    {
        fprintf(stderr, "Error: Operation has too many arguments.\n");
        return OPER_MANY;
    }

    return SUCCESS;
}
ERRCODE rel_dup_check(Rel* r) //Check if relation has duplicate elements
{
    for (size_t i = 0; i < r->cnt; i++)
    {
        bool found = false;
        for (size_t j = 0; j < r->cnt; j++)
        {
            if ( !strcmp(r->v1[i], r->v1[j]) && !strcmp(r->v2[i], r->v2[j]) )
            {
                if (!found)
                    found = true;
                else
                {
                    fprintf(stderr, "\nError [%s] : relation has duplicate elements.", __func__);
                    return ELEM_DUP;
                }
            }
        }
        
    }
    
    return SUCCESS;
}
ERRCODE set_dup_check(Set* s) //Check if set has duplicate elements
{
    for (size_t i = 0; i < s->cnt; i++)
    {
        bool found = false;
        
        for (size_t j = 0; j < s->cnt; j++)
        {
            if ( !strcmp(s->elm[i], s->elm[j]) )
            {
                if (!found)
                    found = true;
                else
                {
                    fprintf(stderr, "\nError [%s] : set has duplicate elements.", __func__);
                    return ELEM_DUP;
                }
            }
        }    
    }
    
    return SUCCESS;
}
ERRCODE noncomm_validate(char* line, Set* uni) //Validates line, not containing command and sets value of passed variables
{
    char* l_copy = strdup(line); //Copies "line" to heap to prevent unintended behaviour
    if (!l_copy) return MEM_ERR;
    ERRCODE err = SUCCESS;
    //WARNING: strtok() modifies passed string!
    char* tk = strtok(l_copy, " "); //First token
    for ( tk = strtok(NULL, " ");  tk != NULL; tk = strtok(NULL, " ") ) //Consequetively extract all tokens from line
    {
        rmv_chars(tk, '(', ')');

        err = uni_check(tk, uni); //Must be SUCCESS(0)!
        if (err)
        {
            FREE(l_copy);
            return err;
        }
    }

    FREE(l_copy);
    return SUCCESS;
}
ERRCODE uni_validate(Set* uni) //Validates univerzum
{
    for (size_t i = 0; i < uni->cnt; i++)
    {
        size_t len = strlen(uni->elm[i]);
        if (len > MAX_ELEM_LEN) //Validates every element's length
        {
            fprintf(stderr, "\nError [%s] : element longer than MAX_ELEM_LEN", __func__);
            return INV_UNI;
        }
        for (size_t j = 0; j < len; j++) //Checks if any element contains invalid characters
        {
            if ( !isalpha(uni->elm[i][j]) )
            {
                fprintf(stderr, "\nError [%s] : character is not alpha", __func__);
                return INV_UNI;
            }
        }
        if ( !strcmp(uni->elm[i], "true") || !strcmp(uni->elm[i], "false") || is_op_name(uni->elm[i])) //Check if univerzum contains special words or operation names
        {
            fprintf(stderr, "\nError [%s] : element is \"true\" or \"false\" or \"operator name\"", __func__);
            return INV_UNI;
        }
    }

    ERRCODE err = SUCCESS;
    if ( (err = set_dup_check(uni)) ) //Check for duplicates in univerzum
        return err;

    return SUCCESS;
}
ERRCODE uni_fill(char* line, Set** uni) //Fills given univerzum with elements from "line" and validates it
{
    char* l_copy = strdup(line); //Copies "line" to heap to prevent unintended behaviour
    if (!l_copy) return MEM_ERR;
    //WARNING: strtok() modifies passed string!
    char* tk = strtok(l_copy, " "); //First token
    //Must be "U"!
    if ( strcmp(tk, "U") )
    {
        fprintf(stderr, "Error: First letter of line is not U");
        return INV_LETTER;
    }

    printf("%s\n", line);

    *uni = set_full(line, 1);
    if (!*uni)
    {
        FREE(l_copy);
        return MEM_ERR;
    }

    ERRCODE error = SUCCESS;
    if ( ( error = uni_validate(*uni) ) )
    {
        FREE(l_copy);
        return error;
    }

    FREE(l_copy);
    return SUCCESS;
}

char* readline(DynaS* ds, FILE* fptr) //Reads the whole line from file using dynamic string "ds"
{    
    ds_clear(ds); //Remove garbage from "ds"
    bool hitEnd = true;
    while (fgets(ds->buff, ds->b_cap, fptr) != NULL)
    {
        ds_bw(ds, ds->buff);
        char c = ds->buff[ds->b_used - 1]; //Last character from buffer
        ds_append(ds);

        if (c == '\n') //If end of line is reached
        {
            hitEnd = false;
            break;
        }
    }
    
    if (!hitEnd) //If end of line is reached
    {
        ds_rmnl(ds);
        return ds->str;
    }
    //If end of file is reached
    ds_rmnl(ds);
    return NULL;
}

//---Mathematical functions implementation
void s_empty(Set* A)
{
    if (!A->cnt)
        printf("true\n");
    else
        printf("false\n");
}
void s_card(Set* A)
{
    printf("%lu\n", A->cnt);
}
bool s_complement(Set* A, Set* uni)
{
    Set* result = set_const();
    if (!result) return NULL;
    
    for (size_t i = 0; i < uni->cnt; i++)
    {
        bool same = false;
        for (size_t j = 0; j < A->cnt; j++)
            if ( !strcmp(uni->elm[i], A->elm[j]) ) //If same
                same = true;
        
        if (!same)
            if (!set_add(result, uni->elm[i]))
            {
                set_dest(result);
                return false;
            }
    }

    set_print(result);
    set_dest(result);
    return true;
}
bool s_union(Set* A, Set* B)
{
    Set* result = set_const();
    if (!result) return NULL;

    for (size_t i = 0; i < A->cnt; i++) //Add all elems from larger set
        if (!set_add(result, A->elm[i]))
        {
            set_dest(result);
            return false;
        }
    for (size_t i = 0; i < B->cnt; i++) //Add other elems from smaller set
    {
        bool same = false;
        for (size_t j = 0; j < result->cnt; j++) 
            if (!strcmp(B->elm[i], result->elm[j]) ) //If same
                same = true;

        if (!same)
            if (!set_add(result, B->elm[i]))
            {
                set_dest(result);
                return false;
            }
    }

    set_print(result);
    set_dest(result);
    return true;
}
bool s_intersect(Set* A, Set* B)
{
    Set* result = set_const();
    if (!result) return NULL;

    for (size_t i = 0; i < A->cnt; i++) //Add elems that are in both sets
    {
        bool same = false;
        for (size_t j = 0; j < B->cnt; j++) //Add elems that are in both sets
            if (!strcmp(A->elm[i], B->elm[j]) ) //If same
                if (!set_add(result, B->elm[j]))
                {
                    set_dest(result);
                    return false;
                }
            
        if (same)
            if (!set_add(result, A->elm[i]))
            {
                set_dest(result);
                return false;
            }
    }

    set_print(result);
    set_dest(result);
    return true;
}
bool s_minus(Set* A, Set* B)
{
    Set* result = set_const();
    if (!result) return NULL;

    for (size_t i = 0; i < A->cnt; i++) //Add elems that are in both sets
    {
        bool same = false;
        for (size_t j = 0; j < B->cnt; j++) //Add elems that are in both sets
            if (!strcmp(A->elm[i], B->elm[j]) ) //If same
                same = true;

        if (!same)
            if (!set_add(result, A->elm[i]))
            {
                set_dest(result);
                return false;
            }
    }

    set_print(result);
    set_dest(result);
    return true;
}
void s_equality(Set* A, Set* B, EQUALITY eq)
{
    if ( (eq == SUBSET && A->cnt >= B->cnt) || (eq == SUBSETEQ && A->cnt > B->cnt) || (eq == EQUAL && A->cnt != B->cnt) )
    {
        printf("false\n");
        return;
    }

    bool allSame = true;
    for (size_t i = 0; i < A->cnt; i++)
    {
        bool same = false;
        for (size_t j = 0; j < B->cnt; j++)
        {
            if (!strcmp(A->elm[i], B->elm[j])) //If same
                same = true;
        }
        if (!same)
            allSame = false;
    }

    if (allSame)
        printf("true\n");
    else
        printf("false\n");
}

//Relation functions
bool is_reflexive(Rel* r, Set* uni)
{
    if (r->cnt == 0)
    {
        if (uni->cnt == 0)
            return true;
        else
            return false;
    }

    bool isReflexive = true;
    for (size_t i = 0; i < r->s->cnt; i++)
    {
        bool has = false;
        for (size_t j = 0; j < r->cnt; j++)
            if ( !strcmp( r->v1[j], r->v2[j] ) && !strcmp( r->v1[j], r->s->elm[i] ))
            {
                has = true;
                //break;
            }

        if (!has)
            isReflexive = false;
    }
    
    return isReflexive;
}
void r_reflexive(Rel* r, Set* uni)
{
    if (is_reflexive(r, uni))
        printf("true\n");
    else
        printf("false\n");
}
bool is_symmetric(Rel* r)
{  
    bool isSymm = true;
 
    for (size_t i = 0; i < r->cnt; i++)
    {
        bool has = false;
        for (size_t j = 0; j < r->cnt; j++)
            if ( !strcmp( r->v1[i], r->v2[j]) && !strcmp( r->v2[i], r->v1[j]) )
            {
                has = true;
                break;
            }
        
        if (!has)
            isSymm = false;
    }
    
    return isSymm;
}
void r_symmetric(Rel* r)
{
    if (is_symmetric(r))
        printf("true\n");
    else
        printf("false\n");
}
void r_antisymmetric(Rel* r)
{
    bool isAsymm = true;
 
    for (size_t i = 0; i < r->cnt; i++)
        for (size_t j = 0; j < r->cnt; j++)
            if ( !strcmp( r->v1[i], r->v2[j]) && !strcmp( r->v2[i], r->v1[j]) )
                if (strcmp( r->v1[i], r->v2[i]))
                    isAsymm = false;
    
    if (isAsymm)
        printf("true\n");
    else
        printf("false\n");
}
bool is_transitive(Rel* r)
{
    bool isTrans = true;

    for (size_t i = 0; i < r->cnt; i++)
    {
        for (size_t j = 0; j < r->cnt; j++)
        {
            if ( !strcmp( r->v2[i], r->v1[j]) )
            {
                isTrans = false;
                for (size_t k = 0; k < r->cnt; k++)
                    if ( !strcmp( r->v1[k], r->v1[i]) && !strcmp( r->v2[k], r->v2[j]))
                        isTrans = true;
            }
        }
    }

    return isTrans;
}
void r_transitive(Rel* r)
{
    if (is_transitive(r))
        printf("true\n");
    else
        printf("false\n");
}
bool is_function(Rel* r, Set* a)
{
    if (a != NULL)
    {
        if (r->cnt != a->cnt)
            return false;
    }

    for (size_t i = 0; i < r->cnt; i++)
    {
        bool found = false;
        for (size_t j = 0; j < r->cnt; j++)
            if ( !strcmp( r->v1[i], r->v1[j] ) )
            {
                if (!found)
                    found = true;
                else
                    return false;
            }
    }

    return true;
}
void r_function(Rel* r)
{
    if (is_function(r, NULL))
        printf("true\n");
    else
        printf("false\n");
}
bool r_domain(Rel* r)
{
    Set* result = set_const();

    for (size_t i = 0; i < r->cnt; i++)
    {
        bool same = false;
        for (size_t j = 0; j < result->cnt; j++)
            if ( !strcmp( r->v1[i], result->elm[j] ) )
                same = true;
        
        if (!same)
            if (!set_add(result, r->v1[i]))
            {
                set_dest(result);
                return false;
            }
    }
    
    set_print(result);
    set_dest(result);
    return true;
}
bool r_codomain(Rel* r)
{
    Set* result = set_const();

    for (size_t i = 0; i < r->cnt; i++)
    {
        bool same = false;
        for (size_t j = 0; j < result->cnt; j++)
            if ( !strcmp( r->v2[i], result->elm[j] ) )
                same = true;
        
        if (!same)
            if (!set_add(result, r->v2[i]))
            {
                set_dest(result);
                return false;
            }
    }
    
    set_print(result);
    set_dest(result);
    return true;
}

bool is_rel_by_sets(Rel* r, Set* a, Set* b)
{
    for (size_t i = 0; i < r->cnt; i++)
    {
        bool has = false;
        for (size_t j = 0; j < a->cnt; j++)
        {    
            if ( !strcmp(r->v1[i], a->elm[j]) )
            {
                has = true;
                break;
            }
        }
        if (!has)
            return false;
    }

    for (size_t i = 0; i < r->cnt; i++)
    {
        bool has = false;
        for (size_t j = 0; j < b->cnt; j++)
        {    
            if ( !strcmp(r->v2[i], b->elm[j]) )
            {
                has = true;
                break;
            }
        }
        if (!has)
            return false;
    }

    return true;
}

bool is_injective(Rel* r, Set* b)
{
    bool isInject = true;
    
    for (size_t i = 0; i < b->cnt; i++)
    {
        bool found = false;
        for (size_t j = 0; j < r->cnt; j++)
        {
            if ( !strcmp( b->elm[i], r->v2[j]) ) 
            {
                if (!found) 
                    found = true;
                else
                {
                    isInject = false;
                    break;
                }
            }       
        }            
    }

    return isInject;
}
void r_injective(Rel* r, Set* a, Set* b)
{
    if (!is_function(r, a))
    {
        printf("false\n");
        return;
    }
    
    if (is_injective(r, b))
        printf("true\n");
    else
        printf("false\n");
}
bool is_surjective(Rel* r, Set* b)
{
    bool isSurject = true;

    for (size_t i = 0; i < b->cnt; i++)
    {
        bool found = false;
        for (size_t j = 0; j < r->cnt; j++)
        {
            if ( !strcmp(b->elm[i], r->v2[j]) )
            {
                found = true;
            }
        }
        if (!found)
        {
            isSurject = false;
            break;
        }
    }

    return isSurject;
}
void r_surjective(Rel* r, Set* a, Set* b)
{
    if (!is_function(r, a))
    {
        printf("false\n");
        return;
    }

    if (is_surjective(r, b))
        printf("true\n");
    else
        printf("false\n");
}
void r_bijective(Rel* r, Set* a, Set* b)
{
    if (!is_function(r, a))
    {
        printf("false\n");
        return;
    }

    if (is_injective(r, b) && is_surjective(r, b))
        printf("true\n");
    else
        printf("false\n");
}

bool rel_cpy(Rel* dst, Rel* src)
{
    for (size_t i = 0; i < src->cnt; i++)
        if (!rel_add(dst, src->v1[i], src->v2[i]))
            return false;

    dst->cap = src->cap;
    dst->cnt = src->cnt;
    dst->index = src->index;
    return true;
}

//Bonus functions
bool closure_ref(Rel* r, Set* uni)
{
    if (is_reflexive(r, uni))
    {
        rel_print(r);
        return true;
    }
    
    Rel* result = rel_const();
    if (!rel_cpy(result, r))
        return false;

    for (size_t i = 0; i < uni->cnt; i++)
    {
        bool found = false;
        for (size_t j = 0; j < r->cnt; j++)
        {
            if ( !strcmp(uni->elm[i], r->v1[j]) && !strcmp(uni->elm[i], r->v2[j]) )
            {
                found = true;
                break;
            }
        }

        if (!found)
            if (!rel_add(result, uni->elm[i], uni->elm[i]))
                return false;
    }
    
    rel_print(result);
    rel_dest(result);
    return true;
}
bool closure_sym(Rel* r)
{
    if (is_symmetric(r))
    {
        rel_print(r);
        return true;
    }

    Rel* result = rel_const();
    if (!rel_cpy(result, r))
        return false;

    for (size_t i = 0; i < r->cnt; i++)
    {
        bool has = false;
        for (size_t j = 0; j < r->cnt; j++)
            if ( !strcmp( r->v1[i], r->v2[j]) && !strcmp( r->v2[i], r->v1[j]) )
            {
                has = true;
                break;
            }
        
        if (!has)
            if (!rel_add(result, r->v2[i], r->v1[i]))
                return false;
    }
    
    rel_print(result);
    rel_dest(result);
    return true;
}
bool closure_trans(Rel* r)
{
    Rel* result = rel_const();
    rel_cpy(result, r);

    while (!is_transitive(result))
    {
        for (size_t i = 0; i < r->cnt; i++)
        {
            for (size_t j = 0; j < r->cnt; j++)
            {
                if ( !strcmp( r->v2[i], r->v1[j]) )
                {
                    bool found = false;
                    for (size_t k = 0; k < r->cnt; k++)
                    {
                        if ( !strcmp( r->v1[k], r->v1[i]) && !strcmp( r->v2[k], r->v2[j]))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        if (!rel_add(result, r->v1[i], r->v2[j]))
                            return false;
                    }
                }
            }
        }
    }
        

    rel_print(result);
    rel_dest(result);
    return true;
}

//Core functions
bool execute_set(char* name, Operation* op) //Executes set functions based on operation name
{
    Set* s = get_set(op->sl, op->index_1);

    if ( !strcmp(name, "empty") )
        s_empty( s );
    else if ( !strcmp(name, "card") )
        s_card( s );
    else if ( !strcmp(name, "complement") )
    {
        if (!s_complement( s, op->uni ))
            return false;
    }
    else if ( !strcmp(name, "union") )
    {
        if (!s_union( s, get_set(op->sl, op->index_2) ))
            return false;
    }
    else if ( !strcmp(name, "intersect") )
    {
        if (!s_intersect( s, get_set(op->sl, op->index_2) ))
            return false;
    }
    else if ( !strcmp(name, "minus") )
    {
        if (!s_minus( s, get_set(op->sl, op->index_2) ))
            return false;
    }
    else if ( !strcmp(name, "subseteq") )
        s_equality( s, get_set(op->sl, op->index_2), SUBSETEQ );
    else if ( !strcmp(name, "subset") )
        s_equality( s, get_set(op->sl, op->index_2), SUBSET );
    else if ( !strcmp(name, "equals") )
        s_equality( s, get_set(op->sl, op->index_2), EQUAL );
    
    return true;
}
ERRCODE execute_rel(char* name, Operation* op) //Executes relation functions based on operation name
{
    Rel* r = get_rel(op->rl, op->index_1);

    if ( !strcmp(name, "reflexive") )
    {
        r_reflexive(r, op->uni);
    }
    else if ( !strcmp(name, "symmetric") )
    {
        r_symmetric(r);
    }
    else if ( !strcmp(name, "antisymmetric") )
    {
        r_antisymmetric(r);
    }
    else if ( !strcmp(name, "transitive") )
    {
        r_transitive(r);
    }
    else if ( !strcmp(name, "function") )
    {
        r_function(r);
    }
    else if ( !strcmp(name, "domain") )
    {
        if (!r_domain(r))
            return MEM_ERR;
    }
    else if ( !strcmp(name, "codomain") )
    {
        if (!r_codomain(r))
            return MEM_ERR;
    }
    else if ( !strcmp(name, "closure_ref") )
    {
        if (!closure_ref(r, op->uni))
            return MEM_ERR;
    }
    else if ( !strcmp(name, "closure_sym") )
    {
        if (!closure_sym(r))
            return MEM_ERR;
    }
    else if ( !strcmp(name, "closure_trans") )
    {
        if (!closure_trans(r))
            return MEM_ERR;
    }
    else
    {
        Set* a = get_set(op->sl, op->index_2);
        Set* b = get_set(op->sl, op->index_3);
        if ( !is_rel_by_sets(r, a, b) )
        {
            printf("false\n");
            return SUCCESS;
        }

        if ( !strcmp(name, "injective") )
        {  
            r_injective(r, a, b);
        }
        else if ( !strcmp(name, "surjective") )
        {
            r_surjective(r, a, b);
        }
        else if ( !strcmp(name, "bijective") )
        {
            r_bijective(r, a, b);
        }
    }

    return SUCCESS;
}
ERRCODE execute(Operation* op) //Execute functions based on opeartion type
{
    bool mode_is_rel = is_op_rel(op->name) ? true : false; //Determine operation mode

    if (mode_is_rel) //If is relation operation
    {
        if (!get_rel(op->rl, op->index_1)) //Try find relation by given index
        {
            fprintf(stderr, "Error: Operator missmatch. : %d\n", __LINE__);
            return OPER_MISS;
        }
        if (is_op_ternary(op->name))
        {
            if (!get_set(op->sl, op->index_2)) //Try find set by given index
            {
                fprintf(stderr, "Error: Operator missmatch. : %d\n", __LINE__);
                return OPER_MISS;
            }

            if (!get_set(op->sl, op->index_3)) //Try find set by given index
            {
                fprintf(stderr, "Error: Operator missmatch. : %d\n", __LINE__);
                return OPER_MISS;
            }
        }
        ERRCODE err;
        if ( (err = execute_rel(op->name, op)) ) //Execute relation operation
            return err;
    }
    else //If is set operation
    {
        if (!get_set(op->sl, op->index_1)) //Try find set by given index
        {
            fprintf(stderr, "Error: Operator missmatch. : %d\n", __LINE__);
            return OPER_MISS;
        }
        if (is_op_binary(op->name))
        {
            if (!get_set(op->sl, op->index_2)) //Try find set by given index
            {
                fprintf(stderr, "Error: Operator missmatch. : %d\n", __LINE__);
                return OPER_MISS;
            }
        }
        
        if (!execute_set(op->name, op)) //Execute relation operation
            return MEM_ERR;
    }

    return SUCCESS;
}
ERRCODE process_comm(Condition* cond, Operation* op) //Processes command line
{
    if (!cond->command_found) //If command not yet found
        cond->command_found = true;
    
    ERRCODE err = SUCCESS;
 
    if ( (err = comm_validate(op)) ) //If command validation failed
        return err;

    if ( (err = execute(op)) )
        return err;
 
    return SUCCESS;
}
ERRCODE process_noncomm(char* line, char* letter_1, size_t line_index, Condition* cond,  Operation* op) //Processes non-command line
{
    if (!cond->set_rel_found) //If set or relation not yet found
            cond->set_rel_found = true;

    if (cond->command_found) //If command already found
    {
        fprintf(stderr, "Error: [%s] [%d] set/relation defined after command.", __func__, INV_SEQUENCE);
        return INV_SEQUENCE;
    }
    ERRCODE err = SUCCESS;
    if ( (err = noncomm_validate(line, op->uni)) ) //Validate line
        return err;
    
    if ( !strcmp(letter_1, "S") ) //If first token of line is "S"
    {
        Set* s = set_full(line, line_index); //Get full set
        if (!s)
            return MEM_ERR;

        if ( (err = set_dup_check(s)) ) //Check for duplicates
            return err;

        setlist_add(op->sl, s); //Add set to list
    }
    else if ( !strcmp(letter_1, "R") ) //If first token of line is "R"
    {
        Rel* r = rel_full(line, line_index); //Get full relation
        if (!r)
            return MEM_ERR;
        
        if ( (err = rel_dup_check(r)) ) //Check for duplicates
            return err;

        rellist_add(op->rl, r); //Add relation to list
    }

    printf("%s\n", line);
    return SUCCESS;
}
ERRCODE process_line(char* line, size_t line_index, Condition* cond,  Operation* op) //Processes a line
{
    if (line[0] == ' ')
    {
        fprintf(stderr, "Error: Empty line in file.");
        return LINE_EMPTY;
    }

    char* l_copy = strdup(line);//Copies "line" to heap to prevent unintended behaviour
    if (!l_copy) return MEM_ERR;
    //WARNING: strtok() modifies passed string!
    char* tk = strtok(l_copy, " "); //First token
    
    if ( strcmp(tk, "S") && strcmp(tk, "R") && strcmp(tk, "C")) //If first token is invalid
    {
        fprintf(stderr, "Error: First letter of line is not S/R/C.");
        FREE(l_copy);
        return INV_LETTER;
    }
    ERRCODE err = SUCCESS;
    if ( strcmp(tk, "C") ) //If not command
    {
        if ( (err = process_noncomm(line, tk, line_index, cond, op)) )
        {
            FREE(l_copy);
            return err;
        }
    }
    else //If command
        if ( (err = process_comm(cond, op)) )
        {
            FREE(l_copy);
            return err;
        }
    
    FREE(l_copy);
    return SUCCESS;
}
void CLEAR_MEMORY(DynaS* ds, Operation* op, FILE** fptr) //Clears all allocated memory and closes file
{
    ds_dest(ds);
    op_dest(op);
    fclose(*fptr);
}
//---Main function
int main(int argc, char** argv)
{
    ERRCODE error = SUCCESS;
    if ( (error = handle_errors(argc)) )
    {
        return error;
    }

    FILE* fptr = fopen(argv[1], "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "Error : failed to open file!");
        fclose(fptr);
        return NO_FILE;
    }

    DynaS* ds = ds_const();
    
    readline(ds, fptr);

    Operation* op = op_const();
    
    if ( (error = uni_fill(ds->str, &op->uni) ) ) //Try get univerzum
    {
        CLEAR_MEMORY(ds, op, &fptr);
        return error;
    }
    setlist_add(op->sl, op->uni); //Add univerzum to set list
    Condition cond = { .set_rel_found = false, .command_found = false };
    // Process other lines
    for(int i = 2; readline(ds, fptr); i++)
    {
        if (i >= MAX_LINE_COUNT + 1)
        {
            fprintf(stderr, "Error [%s]: file has more lines than %d!", __func__, MAX_LINE_COUNT);
            CLEAR_MEMORY(ds, op, &fptr);
            return LINES_MANY;
        }

        if ( (error = process_line(ds->str, i, &cond, op)) )
        {
            CLEAR_MEMORY(ds, op, &fptr);
            return error;
        }
    }

    if ( (error = cond_check(cond)) )
    {
        CLEAR_MEMORY(ds, op, &fptr);
        return error;
    }

    CLEAR_MEMORY(ds, op, &fptr);
    return EXIT_SUCCESS;
}