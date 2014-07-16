CC      = g++
CFLAGS  = -Wall -Wextra -Werror -std=c++11
LDFLAGS = 

all: translation 

translation: translation.o preprocessing.o
	$(CC) -o $@ $^ $(LDFLAGS)

translation.o: translation.cpp translation.h
	$(CC) -c $(CFLAGS) $<

tokenizer.o: tokenizer.cpp tokenizer.h
	$(CC) -c $(CFLAGS) $<

preprocessing.o: preprocessing.cpp preprocessing.h
	$(CC) -c $(CFLAGS) $<
.PHONY: clean cleanest

clean:
	rm *.o

cleanest: clean
	rm translation
