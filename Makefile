CC=gcc
CFLAGS=-Wall -O3

HEADERS = includes.h
CLIENT = c2_client.o
SERVER = backdoor.o

default: backdoor client

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

backdoor: $(SERVER)
	gcc $(SERVER) -o $@ tiny-aes/aes.c --std=gnu99 -static -static-libgcc -static-libstdc++

client: $(CLIENT)
	gcc $(CLIENT) -o $@ tiny-aes/aes.c

clean:
	-rm -f *.o
	-rm -f backdoor client