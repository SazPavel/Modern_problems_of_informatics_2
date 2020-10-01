CC=gcc
IDIR=./include
SRCDIR=./src
CFLAGS=-Wall -I$(IDIR)
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
TARGET=main		
.PHONY: all clean $(TARGET)
default: $(TARGET) clean
all: default
$(SOURCES):
%.o: %.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$< -o $@
$(TARGET): %: %.o $(OBJECTS)
	$(CC) $(OBJECTS) -o $@
clean:
	rm -f *.o
