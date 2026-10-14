#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static unsigned long long memtotal_kb(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        return 0;
    }

    char key[64];
    unsigned long long value = 0;
    char unit[32];

    while (fscanf(fp, "%63s %llu %31s", key, &value, unit) == 3) {
        if (strcmp(key, "MemTotal:") == 0) {
            fclose(fp);
            return value;
        }
    }

    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{
    unsigned long long bytes;

    if (argc == 2) {
        char *end = NULL;
        errno = 0;
        bytes = strtoull(argv[1], &end, 10);
        if (errno != 0 || end == argv[1] || *end != '\0') {
            fprintf(stderr, "usage: %s [bytes]\n", argv[0]);
            return 2;
        }
    } else {
        unsigned long long kb = memtotal_kb();
        if (kb == 0) {
            fprintf(stderr, "cannot read MemTotal\n");
            return 1;
        }
        bytes = kb * 1024ull * 10ull;
    }

    void *ptr = mmap(NULL, (size_t)bytes, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        printf("mmap=failed bytes=%llu errno=%d error=%s\n",
               bytes, errno, strerror(errno));
        return errno == ENOMEM ? 12 : 1;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    size_t touched = (size_t)page_size * 4;
    if (touched > bytes) {
        touched = (size_t)bytes;
    }

    volatile char *p = ptr;
    for (size_t i = 0; i < touched; i += (size_t)page_size) {
        p[i] = 42;
    }

    printf("mmap=ok bytes=%llu touched=%zu\n", bytes, touched);
    munmap(ptr, (size_t)bytes);
    return 0;
}
