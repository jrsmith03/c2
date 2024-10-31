CC=gcc
CFLAGS=-Wall -O3

HEADERS = includes.h
CLIENT = client.o
SERVER = server.o

default: server client

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

server: $(SERVER)
	gcc $(SERVER) -o $@ tiny-aes/aes.c

client: $(CLIENT)
	gcc $(CLIENT) -o $@ tiny-aes/aes.c

clean:
	-rm -f *.o
	-rm -f server client