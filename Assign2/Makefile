all: client server

client: client.c
	gcc -lm client.c -lcrypto -o wbc379
	
server: server.c
	gcc -lm -m32 -std=gnu99 server.c -lpthread -o wbs379

clean:
	rm -f wbc379 wbs379
