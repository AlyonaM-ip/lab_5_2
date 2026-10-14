#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s mib step_delay_ms\n", argv[0]);
        return 2;
    }

    size_t mib = (size_t)strtoull(argv[1], NULL, 10);
    long delay_ms = strtol(argv[2], NULL, 10);
    size_t size = mib * 1024ull * 1024ull;
    long page_size = sysconf(_SC_PAGESIZE);

    uint8_t *region = mmap(NULL, size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("pid=%ld target_mib=%zu\n", (long)getpid(), mib);
    fflush(stdout);

    struct timespec pause = {
        .tv_sec = delay_ms / 1000,
        .tv_nsec = (delay_ms % 1000) * 1000000L,
    };

    for (size_t off = 0; off < size; off += (size_t)page_size) {
        region[off] = (uint8_t)(off >> 12);
        if (delay_ms > 0 && (off % (1024ull * 1024ull)) == 0) {
            nanosleep(&pause, NULL);
        }
    }

    printf("pid=%ld filled_mib=%zu sleeping\n", (long)getpid(), mib);
    fflush(stdout);

    for (;;) {
        sleep(60);
    }
}
