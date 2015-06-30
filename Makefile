
CC=cc
CFLAGS=-c -std=c11 -Wall -Wextra -Wpedantic -D DEBUG=0
LDFLAGS=
SHARED_SOURCES=daemon.c server_select.c local_control.c
SHARED_OBJECTS=$(SHARED_SOURCES:.c=.o)
EXECUTABLES=relay_chat

all: $(EXECUTABLES)
    
relay_chat: $(SHARED_OBJECTS) protocol_relay_chat.o
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(EXECUTABLES)

