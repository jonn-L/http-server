CC = gcc
CFLAGS = -Wall -Werror

all: server

server: server.o message_handle_tools.o
	$(CC) $(CFLAGS) -o server server.o message_handle_tools.o

server.o: server.c message_handle_tools.h
	$(CC) $(CFLAGS) -c server.c

message_handle_tools.o: message_handle_tools.c message_handle_tools.h
	$(CC) $(CFLAGS) -c message_handle_tools.c

clean:
	rm -f *.o server
