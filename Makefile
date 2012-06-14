CFLAGS=-c -Werror -Wextra -Wall -ggdb3
CC=gcc
LDFLAGS=
SOURCES=die.c listen_server.c event_loop.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=shackle

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
