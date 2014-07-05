CC      = g++
CFLAGS  = -Wall -Wextra -Werror -std=c++11
LDFLAGS = 

all: tokenizer clean

tokenizer: tokenizer.o
	$(CC) -o $@ $^ $(LDFLAGS)

tokenizer.o: tokenizer.cpp tokenizer.h
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm *.o

cleanest: clean
	rm tokenizer
