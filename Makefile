
## Created by Anjuta

CC = gcc
CFLAGS = -g -Wall
OBJECTS = wiiarmed.o owiarm.o
INCFLAGS = -I /opt/local/include
LDFLAGS = -L /opt/local/lib
LIBS = -lusb-1.0 -lwiiuse

all: wiiarmed

wiiarmed: $(OBJECTS)
	$(CC) -o wiiarmed $(OBJECTS) $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

.c.o :
	$(CC) -o $@ -c $(CFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean:
	rm -f *.o

.PHONY: all
.PHONY: count
.PHONY: clean
