CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11

all: overcommit_test thp_bench oom_alloc

overcommit_test: overcommit_test.c
	$(CC) $(CFLAGS) -o $@ $<

thp_bench: thp_bench.c
	$(CC) $(CFLAGS) -o $@ $<

oom_alloc: oom_alloc.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f overcommit_test thp_bench oom_alloc

.PHONY: all clean
