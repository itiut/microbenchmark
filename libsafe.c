#define _LARGEFILE64_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "libsafe.h"

int safe_open(const char *pathname, int flags) {
    int fd = open(pathname, flags);
    if (fd == -1) {
        perror("safe_open: open(2)");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void safe_ioctl(int d, int request, void *ptr) {
    int ret = ioctl(d, request, ptr);
    if (ret == -1) {
        perror("safe_ioctl: ioctl(2)");
        exit(EXIT_FAILURE);
    }
}

ssize_t safe_read(int fd, void *buf, size_t count) {
    ssize_t size = read(fd, buf, count);
    if (size == -1) {
        perror("safe_read: read(2)");
        exit(EXIT_FAILURE);
    }
    return size;
}

ssize_t safe_write(int fd, const void *buf, size_t count) {
    ssize_t size = write(fd, buf, count);
    if (size == -1) {
        perror("safe_write: write(2)");
        exit(EXIT_FAILURE);
    }
    return size;
}

void safe_fsync(int fd) {
    int ret = fsync(fd);
    if (ret == -1) {
        perror("safe_fsync: fsync(2)");
        exit(EXIT_FAILURE);
    }
}

void safe_lseek64(int fd, long long offset, int whence) {
    off64_t ret = lseek64(fd, offset, whence);
    if (ret == -1) {
        perror("safe_lseek64: lseek64(3)");
        exit(EXIT_FAILURE);
    }
}

void *safe_calloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (ptr == NULL) {
        perror("safe_calloc: calloc(3)");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void safe_gettimeofday(struct timeval *tv) {
    if (gettimeofday(tv, NULL) == -1) {
        perror("gettimeofday(2)");
        exit(EXIT_FAILURE);
    }
}
