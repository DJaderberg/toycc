CC      = g++
CFLAGS  = -Wall -Wextra -Werror -std=c++11
LDFLAGS = 

all: translation clean

translation: translation.o 
	$(CC) -o $@ $^ $(LDFLAGS)

translation.o: translation.cpp translation.h
	$(CC) -c $(CFLAGS) $<

tokenizer.o: tokenizer.cpp tokenizer.h
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm *.o

cleanest: clean
	rm translation
