#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

int unzip_deflate(unsigned char *in, int insz, unsigned char *out, int outsz, int no_error) {
    static z_stream *z  = NULL;
    int     ret,
            sync = Z_FINISH;
   
    if(!in && !out) {
        if(z) {
            inflateEnd(z);
            free(z);
        }
        z = NULL;
        return -1;
    }
   
    if(!z) {
        z = calloc(sizeof(z_stream), 1);
        if(!z) return -1;
        if(inflateInit2(z, -15)) {
            fprintf(stderr, "\nError: zlib initialization error\n");
            return -1;
        }
    }
    inflateReset(z);

    z->next_in   = in;
    z->avail_in  = insz;
    z->next_out  = out;
    z->avail_out = outsz;
    ret = inflate(z, sync);
    if((ret != Z_STREAM_END) && !no_error) {
        fprintf(stderr, "\nError: the compressed zlib/deflate input is wrong or incomplete (%d)\n", ret);
        return -1;
    }
    return z->total_out;
}

static unsigned short GetShort(FILE *file)
{
	const unsigned char byte = fgetc(file);
	return (byte << 8) | fgetc(file);
}

static unsigned long GetLong(FILE *file)
{
	const unsigned char byte1 = fgetc(file);
	const unsigned char byte2 = fgetc(file);
	const unsigned char byte3 = fgetc(file);
	return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | fgetc(file);
}

int main(int argc, char *argv[])
{
	if (argc >= 1)
	{
		FILE *in_file = fopen(argv[1], "rb");

		if (in_file)
		{
			FILE *out_file = fopen("out.ar.00", "wb");

			char fourcc[4];
			fread(fourcc, 4, 1, in_file);

			if (!memcmp(fourcc, "segs", 4))
			{
				const unsigned short unknown = GetShort(in_file);
				printf("Unknown -> 0x%X\n", unknown);
				const unsigned short chunk_count = GetShort(in_file);
				printf("Chunk count -> 0x%X\n", chunk_count);
				const unsigned long uncompressed_size = GetLong(in_file);
				const unsigned long compressed_size = GetLong(in_file);

				for (unsigned int i = 0; i < chunk_count; ++i)
				{
					const unsigned short chunk_compressed_size = GetShort(in_file);
					printf("Chunk compressed size -> 0x%X\n", chunk_compressed_size);
					unsigned long chunk_uncompressed_size = GetShort(in_file);
					if (chunk_uncompressed_size == 0)
						chunk_uncompressed_size = 0x10000;
					printf("Chunk uncompressed size -> 0x%lX\n", chunk_uncompressed_size);
					const unsigned long chunk_offset = GetLong(in_file);
					printf("Chunk offset -> 0x%lX\n", chunk_offset);
					const long position = ftell(in_file);
					fseek(in_file, chunk_offset - 1, SEEK_SET);

					unsigned char *uncompressed_buffer = malloc(chunk_uncompressed_size);
					unsigned char *compressed_buffer = malloc(chunk_compressed_size);
					fread(compressed_buffer, 1, chunk_compressed_size, in_file);

					unzip_deflate(compressed_buffer, chunk_compressed_size, uncompressed_buffer, chunk_uncompressed_size, 0);

					fwrite(uncompressed_buffer, 1, chunk_uncompressed_size, out_file);

					free(uncompressed_buffer);
					free(compressed_buffer);

					fseek(in_file, position, SEEK_SET);
				}
			}
			else
			{
				printf("File is not a segs archive\n");
			}

			unzip_deflate(NULL, 0, NULL, 0, 0);	// Deinit
			fclose(in_file);
			fclose(out_file);
		}
		else
		{
			printf("Could not open input file\n");
		}
	}
	else
	{
		printf("No input file\n");
	}
}
