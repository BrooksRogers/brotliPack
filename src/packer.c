#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <brotli/encode.h>

#define CHUNK 16384

void compress_file(const char *source, const char *dest)
{
    FILE *source_file = fopen(source, "rb");
    if (!source_file)
    {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    FILE *dest_file = fopen(dest, "wb");
    if (!dest_file)
    {
        perror("Failed to open destination file");
        fclose(source_file);
        exit(EXIT_FAILURE);
    }

    BrotliEncoderState *s = BrotliEncoderCreateInstance(NULL, NULL, NULL);
    if (s == NULL)
    {
        perror("Failed to create Brotli encoder instance");
        fclose(source_file);
        fclose(dest_file);
        exit(EXIT_FAILURE);
    }

    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    size_t available_in;
    const unsigned char *next_in;
    size_t available_out;
    unsigned char *next_out;
    BrotliEncoderOperation op;

    do
    {
        available_in = fread(in, 1, CHUNK, source_file);
        if (ferror(source_file))
        {
            perror("Error reading source file");
            BrotliEncoderDestroyInstance(s);
            fclose(source_file);
            fclose(dest_file);
            exit(EXIT_FAILURE);
        }
        next_in = in;
        op = feof(source_file) ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;

        do
        {
            available_out = CHUNK;
            next_out = out;
            if (!BrotliEncoderCompressStream(s, op, &available_in, &next_in, &available_out, &next_out, NULL))
            {
                perror("Brotli compression failed");
                BrotliEncoderDestroyInstance(s);
                fclose(source_file);
                fclose(dest_file);
                exit(EXIT_FAILURE);
            }
            size_t out_size = CHUNK - available_out;
            if (fwrite(out, 1, out_size, dest_file) != out_size || ferror(dest_file))
            {
                perror("Error writing to destination file");
                BrotliEncoderDestroyInstance(s);
                fclose(source_file);
                fclose(dest_file);
                exit(EXIT_FAILURE);
            }
        } while (BrotliEncoderHasMoreOutput(s));
    } while (op != BROTLI_OPERATION_FINISH);

    BrotliEncoderDestroyInstance(s);
    fclose(source_file);
    fclose(dest_file);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source file> <destination file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    compress_file(argv[1], argv[2]);

    return EXIT_SUCCESS;
}