CFLAGS=-c -Wextra -Wall -ggdb3 -std=c99
CC=gcc
LDFLAGS=-levent -ljson
SOURCES=die.c event_server.c message_handler.c job_manager.c job.c report_handler.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=shackle

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
