LOGIN=xgonce00
BIN1=tail
BIN2=wordcount
CPPBIN=wordcount-

CODEDIRS=.
INCDIRS=.

CC=gcc
OPT=-O4

PLATFORM=-m64

WARNINGS=-Wall -Wextra -pedantic
CFLAGS=-std=c11 -g $(WARNINGS) $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(PLATFORM)
LDFLAGS=

CPPFILES=wordcount-.cc
CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))

LIB=libhtab
OBJ1 = $(BIN1).o
OBJLIB = io.o htab_find.o htab_erase.o htab_lookup_add.o htab_free.o\
	htab_clear.o htab_resize.o htab_size_bucket.o htab_hash_function.o htab_init.o htab_for_each.o htab_private.o
OBJLIB-sh = $(patsubst %.o, %-sh.o, $(OBJLIB))

COMPILE_BIN=$(CC) $(PLATFORM) $< -o $(strip $@ $(LDFLAGS))

.PHONY: all dep clean zip
all: $(BIN1) $(BIN2) $(BIN2)-dynamic wordcount- dep.list

$(BIN1): $(OBJ1)
	$(COMPILE_BIN)

$(BIN2): $(BIN2).o $(LIB).a 
	$(CC) -o $@ $^

$(BIN2)-dynamic: $(LIB).so
	$(CC) $(BIN2).c -Wl,-rpath . -L. -lhtab -o $@

$(CFILES) $(CPPFILES): Makefile
	@touch $@

wordcount-: wordcount-.o
	g++ $(PLATFORM) $^ -o $@

wordcount-.o: wordcount-.cc
	g++ -std=c++17 -pedantic -Wall -O2 -c $< -o $@

libhtab.a: $(OBJLIB)
	ar rcs $@ $^

libhtab.so: $(OBJLIB-sh)
	$(CC) $^ -shared -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%-sh.o: %.c
	$(CC) $(CFLAGS) -c -fPIC $< -o $@

dep.list:
	$(CC) -MM *.c >dep.list

clean:
	rm -rf $(BIN1) $(BIN2) $(BIN2)-dynamic $(CPPBIN) $(LOGIN).zip *.o *.d *.a *.so

zip:
	zip $(LOGIN).zip *.c *.cc *.h Makefile

-include dep.list