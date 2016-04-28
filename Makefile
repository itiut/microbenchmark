PROGRAM := microbench
IGNORES :=
SOURCES := $(filter-out $(IGNORES), $(wildcard *.c))
OBJECTS := $(SOURCES:.c=.o)

CC := gcc
CFLAGS := -Wall -O2 -std=c99
LDLIBS := -lrt

.SUFFIXES: .c .o

.c.o:

.PHONY: all clean

all: $(PROGRAM) $(OBJECTS)

$(PROGRAM): $(OBJECTS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
