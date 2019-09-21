CFLAGS = -O3 -s -std=c99 -fno-ident -Wall -Wextra -pedantic
LDFLAGS = -lz

all: decompress compress

decompress: decompress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

compress: compress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
