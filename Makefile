CC=cc
CFLAGS=-c -std=c11 -Wall -Wextra -Wpedantic -D DEBUG=0
LDFLAGS=
SHARED_SOURCES=daemon.c args.c log.c server_select.c local_control.c
SHARED_OBJECTS=$(SHARED_SOURCES:.c=.o)
EXECUTABLES=relay_chat args_test

all: $(EXECUTABLES)
    
relay_chat: $(SHARED_OBJECTS) protocol_relay_chat.o
	$(CC) $(LDFLAGS) $^ -o $@

args_test: args_test.o
	$(CC) $(LDFLAGS) $^ -o $@

args_test.o: args.c
	$(CC) $(CFLAGS) -D ARGS_MAIN=1 $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(EXECUTABLES)

