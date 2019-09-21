CFLAGS = -O3 -s -std=c11 -fno-ident -Wall -Wextra -Wno-unused-variable
LDFLAGS = -lz

all: decompress compress

decompress: decompress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

compress: compress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
