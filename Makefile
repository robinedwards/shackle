CFLAGS=-c -Wextra -Wall -ggdb3
CC=gcc
LDFLAGS=-levent
SOURCES=die.c event_server.c message_handler.c job.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=shackle

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
