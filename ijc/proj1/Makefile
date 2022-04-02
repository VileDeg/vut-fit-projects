BIN1=primes
BIN2=steg-decode

CODEDIRS=.
INCDIRS=.

CC=gcc
OPT=-O4
PLATFORM=-m64

WARNINGS=-Wall -Wextra -pedantic
CFLAGS=-std=c11 -g $(WARNINGS) $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(PLATFORM)
LDFLAGS=-lm

CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))

HEADERS=bitset.h error.h ppm.h

OBJSHARED=eratosthenes.o error.o

OBJLIST1=$(OBJSHARED) primes.o
OBJLIST1-i=primes-i.o eratosthenes-i.o bitset-i.o error.o
OBJLIST2=$(OBJSHARED) steg-decode.o ppm.o

COMPILE_BIN=$(CC) $(PLATFORM) $^ -o $(strip $@ $(LDFLAGS))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all run dep clean zip
all: $(BIN1) $(BIN1)-i $(BIN2)

run: $(BIN1) $(BIN1)-i
	./$(BIN1)
	./$(BIN1)-i

dep: 
	$(CC) -MM *.c >dep.list

-include dep.list

$(BIN1): $(OBJLIST1)
	$(COMPILE_BIN)
	
$(BIN1)-i: $(OBJLIST1-i)
	$(COMPILE_BIN)

$(BIN2): $(OBJLIST2)
	$(COMPILE_BIN)

$(filter %-i.o, $(OBJLIST1-i)): %-i.o: %.c
	$(CC) $(CFLAGS) -DUSE_INLINE -c $< -o $@

$(CFILES): makefile
	@touch $@

clean:
	rm -rf $(BIN1) $(BIN1)-i $(BIN2) *.o *.d

zip:
	zip xgonce00.zip *.c *.h Makefile