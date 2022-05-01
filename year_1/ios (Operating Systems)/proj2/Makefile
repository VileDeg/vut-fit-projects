NAME=proj2

CC=gcc
#-Werror 
CFLAGS=-g -std=gnu99 -Wall -Wextra -pedantic
LDFLAGS=-lpthread -lrt

.PHONY: all clean zip

all: $(NAME)

$(NAME): $(NAME).o
	gcc $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(NAME).c: Makefile
	@touch $@

clean:
	rm -rf $(NAME) *.o

zip:
	zip proj2.zip *.c *.h Makefile