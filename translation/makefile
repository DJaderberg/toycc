CC      = g++
CFLAGS  = -Wall -Wextra -std=c++11
LDFLAGS = 
OBJECTS = translation.o preprocessing.o source.o syntax.o
all: translation 

translation: $(OBJECTS)
	$(CC) -o translation $^ $(LDFLAGS)

syntax.o : syntax.cpp syntax.h abstractSyntax.h
	$(CC) -c $(CFLAGS) $<

%.o: %.cpp %.h
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm *.o

cleanest: clean
	rm translation
