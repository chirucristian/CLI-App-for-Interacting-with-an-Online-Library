CC=gcc
CFLAGS=-Wall

SOURCES=client.c requests.c helpers.c buffer.c parson.c

build:
	$(CC) -o client $(CFLAGS) $(SOURCES)

run: build
	./client

clean:
	rm -f client
