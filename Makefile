CFLAGS = -O3 -s -static -std=c11 -fomit-frame-pointer -fno-ident -Wall -Wextra -Wno-unused-variable
LDFLAGS = -lz

all: decompress.exe compress.exe

decompress.exe: decompress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

compress.exe: compress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
