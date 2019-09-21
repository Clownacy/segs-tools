#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

// This function is copied from QuickBMS
int deflate_compress(unsigned char *in, int insz, unsigned char *out, int outsz)
{
	const bool no_error = false;

	static z_stream *stream;
   
	if(!in && !out)
	{
		if(stream)
		{
			deflateEnd(stream);
			free(stream);
			stream = NULL;
		}

		return -1;
	}
   
	if(!stream)
	{
		stream = calloc(sizeof(z_stream), 1);

		if(!stream)
			return -1;

		if(deflateInit2(stream, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY))
		{
			fprintf(stderr, "\nError: zlib initialization error\n");
			return -1;
		}
	}

	deflateReset(stream);

	stream->next_in   = in;
	stream->avail_in  = insz;
	stream->next_out  = out;
	stream->avail_out = outsz;
	const int ret = deflate(stream, Z_FINISH);

	if(ret != Z_STREAM_END && !no_error)
	{
		fprintf(stderr, "\nError: the compressed zlib/deflate input is wrong or incomplete (%d)\n", ret);
		return -1;
	}

	return stream->total_out;
}

static void PutShort(FILE *file, unsigned short value)
{
	fputc(value >> 8, file);
	fputc(value, file);
}

static void PutLong(FILE *file, unsigned long value)
{
	fputc(value >> 24, file);
	fputc(value >> 16, file);
	fputc(value >> 8, file);
	fputc(value, file);
}

int main(int argc, char *argv[])
{
	if (argc >= 1)
	{
		FILE *in_file = fopen(argv[1], "rb");

		if (in_file)
		{
			fseek(in_file, 0, SEEK_END);
			const long in_file_size = ftell(in_file);
			rewind(in_file);

			const unsigned int chunk_count = ((in_file_size - 1) / 0x10000) + 1;

			FILE *out_file = fopen("out.ar.00", "wb");

			fseek(out_file, 0x10 + (chunk_count * 8), SEEK_SET);

			unsigned char *src_buffer = malloc(0x10000);
			unsigned char *dst_buffer = malloc(0x10000);

			for (long i = 0; i < chunk_count - 1; ++i)
			{
				const long data_pos = ftell(out_file);
				fread(src_buffer, 1, 0x10000, in_file);
				const int bytes = deflate_compress(src_buffer, 0x10000, dst_buffer, 0x10000);
				fwrite(dst_buffer, 1, bytes, out_file);

				size_t bytes_remaining = -ftell(out_file) & 0xF;
				for (unsigned int i = 0; i < bytes_remaining; ++i)
					fputc(0, out_file);

				const long last_pos = ftell(out_file);

				// Fill in header
				fseek(out_file, 0x10 + (i * 8), SEEK_SET);
				PutShort(out_file, bytes);
				PutShort(out_file, 0);
				PutLong(out_file, data_pos + 1);
				fseek(out_file, last_pos, SEEK_SET);
			}

			if (in_file_size & 0xFFFF)
			{
				const long data_pos = ftell(out_file);
				const long size = in_file_size & 0xFFFF;
				fread(src_buffer, 1, size, in_file);
				const int bytes = deflate_compress(src_buffer, size, dst_buffer, 0x10000);
				fwrite(dst_buffer, 1, bytes, out_file);

				size_t bytes_remaining = -ftell(out_file) & 0xF;
				for (unsigned int i = 0; i < bytes_remaining; ++i)
					fputc(0, out_file);

				const long last_pos = ftell(out_file);

				// Fill in header
				fseek(out_file, 0x10 + ((chunk_count - 1) * 8), SEEK_SET);
				PutShort(out_file, bytes);
				PutShort(out_file, size);
				PutLong(out_file, data_pos + 1);
				fseek(out_file, last_pos, SEEK_SET);
			}

			const unsigned long compressed_size = ftell(out_file);

			rewind(out_file);

			fputs("segs", out_file);
			PutShort(out_file, 4);
			PutShort(out_file, chunk_count);
			PutLong(out_file, in_file_size);
			PutLong(out_file, compressed_size);

			deflate_compress(NULL, 0, NULL, 0);	// Deinit
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
