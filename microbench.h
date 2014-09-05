#ifndef MICROBENCH_H
#define MICROBENCH_H

typedef enum bench_type_t {
    SEQ_RD  = 0,
    SEQ_WR  = 1,
    RAND_RD = 2,
    RAND_WR = 3,
    BENCH_TYPE_NONE,
    BENCH_TYPE_ERROR
} bench_type_t;

void usage(const char *program_name);

bench_type_t bench_name_to_type(const char *type_name);

int open_device(const char *device, bench_type_t bench_type);

void run(int fd, const char *device, bench_type_t bench_type, long long each_bytes, long long max_count, bool is_verbose);

double timeval_to_f(struct timeval tv);

long long llrandom(long long min, long long max);

#endif /* MICROBENCH_H */
