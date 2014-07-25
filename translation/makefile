CC      = g++
CFLAGS  = -Wall -Wextra -Werror -std=c++11
LDFLAGS = 
OBJECTS = translation.o preprocessing.o source.o syntax.o
all: translation 

translation: $(OBJECTS) 
	$(CC) -o translation $^ $(LDFLAGS)

%.o: %.cpp %.h
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm *.o

cleanest: clean
	rm translation
