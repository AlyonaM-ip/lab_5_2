#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static uint64_t nsec_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t xorshift64(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

int main(int argc, char **argv)
{
    size_t mib = 1024;
    const char *mode = "default";

    if (argc >= 2) {
        mib = (size_t)strtoull(argv[1], NULL, 10);
    }
    if (argc >= 3) {
        mode = argv[2];
    }

    size_t size = mib * 1024ull * 1024ull;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf");
        return 1;
    }

    uint8_t *region = mmap(NULL, size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    if (strcmp(mode, "huge") == 0) {
        if (madvise(region, size, MADV_HUGEPAGE) != 0) {
            perror("madvise MADV_HUGEPAGE");
        }
    } else if (strcmp(mode, "nohuge") == 0) {
        if (madvise(region, size, MADV_NOHUGEPAGE) != 0) {
            perror("madvise MADV_NOHUGEPAGE");
        }
    }

    for (size_t off = 0; off < size; off += (size_t)page_size) {
        region[off] = (uint8_t)off;
    }

    size_t pages = size / (size_t)page_size;
    size_t iterations = pages * 16;
    uint64_t seed = 0x123456789abcdef0ull;
    volatile uint64_t sum = 0;

    uint64_t start = nsec_now();
    for (size_t i = 0; i < iterations; i++) {
        size_t page = (size_t)(xorshift64(&seed) % pages);
        sum += region[page * (size_t)page_size];
    }
    uint64_t finish = nsec_now();

    printf("mib=%zu mode=%s iterations=%zu elapsed_ms=%.3f checksum=%llu\n",
           mib, mode, iterations, (double)(finish - start) / 1000000.0,
           (unsigned long long)sum);

    munmap(region, size);
    return 0;
}
