#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <fcntl.h>
#include <getopt.h>
#include <linux/fs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "libsafe.h"
#include "microbench.h"

static const long long DEFAULT_EACH_BYTES = 512;
static const long long DEFAULT_COUNT = 100000;
static const bench_type_t DEFAULT_BENCH_TYPE = SEQ_RD;

static const struct option longopts[] = {
    {"bs",      required_argument, NULL, 'b'},
    {"count",   required_argument, NULL, 'c'},
    {"help",    no_argument,       NULL, 'h'},
    {"type",    required_argument, NULL, 't'},
    {"verbose", no_argument,       NULL, 'v'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    const char *program_name = argv[0];
    const char *device = "";
    long long each_bytes = -1;
    long long count = -1;
    bench_type_t bench_type = BENCH_TYPE_NONE;
    bool is_verbose = false;

    int opt;
    while ((opt = getopt_long(argc, argv, "b:c:ht:v", longopts, NULL)) != -1) {
        switch (opt) {
        case 'b':
            each_bytes = atoll(optarg);
            if (each_bytes <= 0) {
                printf("%s: incorrect bs operand BYTES\n", program_name);
                exit(EXIT_FAILURE);
            }
            break;
        case 'c':
            count = atoll(optarg);
            if (count <= 0) {
                printf("%s: incorrect count operand N\n", program_name);
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            usage(program_name);
            exit(EXIT_SUCCESS);
        case 't':
            bench_type = bench_name_to_type(optarg);
            if (bench_type == BENCH_TYPE_ERROR) {
                printf("%s: incorrect type operand TYPE\n", program_name);
                exit(EXIT_FAILURE);
            }
            break;
        case 'v':
            is_verbose = true;
            break;
        case '?':
            usage(program_name);
            exit(EXIT_FAILURE);
        }
    }
    if (optind == argc) {
        printf("%s: missing device operand\n", program_name);
        exit(EXIT_FAILURE);
    }
    device = argv[optind];

    /* default options */
    if (each_bytes == -1) {
        each_bytes = DEFAULT_EACH_BYTES;
    }
    if (count == -1) {
        count = DEFAULT_COUNT;
    }
    if (bench_type == BENCH_TYPE_NONE) {
        bench_type = DEFAULT_BENCH_TYPE;
    }

    srand(time(NULL));

    int fd = open_device(device, bench_type);
    run(fd, device, bench_type, each_bytes, count, is_verbose);
    close(fd);
    return 0;
}

void usage(const char *program_name) {
    printf("Usage: %s [OPTION]... DEVICE\n", program_name);
    printf("Run micro benchmark on DEVICE according to the options.\n");
    printf("\n");
    printf("  -b, --bs=BYTES   read and write up to BYTES bytes at a time\n");
    printf("  -c, --count=N    read and write up to N blocks or EOF\n");
    printf("  -h, --help       display this help and exit\n");
    printf("  -t, --type=TYPE  set read and write types to TYPES:\n");
    printf("                     SEQ_RD, SEQ_WR, RAND_RD, RAND_WR\n");
}

static const char *type_names[] = {
    "SEQ_RD",
    "SEQ_WR",
    "RAND_RD",
    "RAND_WR"
};

bench_type_t bench_name_to_type(const char *type_name) {
    for (size_t i = 0; i < sizeof(type_names) / sizeof(type_names[0]); i++) {
        if (strncmp(type_names[i], type_name, strlen(type_names[i])) == 0) {
            return i;
        }
    }
    return BENCH_TYPE_ERROR;
}

int open_device(const char *device, bench_type_t bench_type) {
    if (bench_type & 0x1) {
        /* write */
        if (strncmp(device, "/dev/sda", strlen("/dev/sda")) == 0) {
            printf("You should not open /dev/sda* for write benchmark\n");
            exit(EXIT_FAILURE);
        }
        return safe_open(device, O_WRONLY);
    }
    /* read */
    return safe_open(device, O_RDONLY);
}

void run(int fd, const char *device, bench_type_t bench_type, long long each_bytes, long long count, bool is_verbose) {
    long long sector_size = 0;
    safe_ioctl(fd, BLKSSZGET, &sector_size);
    long long n_of_sectors = 0;
    safe_ioctl(fd, BLKGETSIZE, &n_of_sectors);
    long long volume = sector_size * n_of_sectors;

    long long count_upper_limit = (volume / each_bytes) + ((volume % each_bytes) ? 1 : 0);
    if (count > count_upper_limit) {
        count = count_upper_limit;
    }

    char *buffer = (char *) safe_calloc(each_bytes, sizeof(char));
    struct timespec *begin_tss = (struct timespec *) calloc(count, sizeof(struct timespec));
    struct timespec *end_tss = (struct timespec *) calloc(count, sizeof(struct timespec));

    for (long long i = 0; i < count; i++) {
        safe_clock_gettime(CLOCK_REALTIME, &begin_tss[i]);
        if (bench_type & 0x2) {
            /* random */
            long long offset = llrandom(0, count - 1) * each_bytes;
            safe_lseek64(fd, offset, SEEK_SET);
        }
        if (bench_type & 0x1) {
            /* write */
            safe_write(fd, buffer, each_bytes);
            safe_fsync(fd);
        } else {
            /* read */
            safe_read(fd, buffer, each_bytes);
        }
        safe_clock_gettime(CLOCK_REALTIME, &end_tss[i]);
    }

    double elapsed_time_sec = 0;
    for (long long i = 0; i < count; i++) {
        elapsed_time_sec += timespec_to_f(end_tss[i]) - timespec_to_f(begin_tss[i]);
    }
    double total_size_mb = each_bytes * count * 1e-6;
    double throughput_mb_per_sec = total_size_mb / elapsed_time_sec;

    printf("####################################\n");
    printf("# Device Information\n");
    printf("#   device name     %s\n",          device);
    printf("#   sector size     %lld bytes\n",  sector_size);
    printf("#   # of sectors    %lld\n",        n_of_sectors);
    printf("#   volume          %.3f GiB\n",    (double) volume / (1 << 30));
    printf("#\n");
    printf("# Benchmark Information\n");
    printf("#   type            %s\n",          type_names[bench_type]);
    printf("#   block size      %lld bytes\n",  each_bytes);
    printf("#   count           %lld\n",        count);
    printf("#   total size      %.3f MB\n",     total_size_mb);
    printf("#   elapsed time    %f sec\n",      elapsed_time_sec);
    printf("#   throughput      %.3f MB/sec\n", throughput_mb_per_sec);
    printf("#   IOPS            %.3f\n",        count / elapsed_time_sec);
    printf("#   latency (mean)  %f msec\n",     elapsed_time_sec * 1e3 / count);
    printf("####################################\n");

    if (is_verbose) {
        printf("#count total_latency[msec]\n");
        long long display_interval = (count > 1000) ? count / 1000 : 1;
        double total = 0;
        for (long long i = 0; i < count; i++) {
            total += timespec_to_f(end_tss[i]) - timespec_to_f(begin_tss[i]);
            if ((i + 1) % display_interval == 0) {
                printf("%lld %f\n", i + 1, total * 1e3);
                total= 0;
            }
        }
    }

    free(buffer);
    free(begin_tss);
    free(end_tss);
}

double timespec_to_f(struct timespec ts) {
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

long long llrandom(long long min, long long max) {
    return min + (long long) ((max - min + 1.0) * (double) rand() / (1.0 + RAND_MAX));
}
