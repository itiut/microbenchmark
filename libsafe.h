#ifndef LIBSAFE_H
#define LIBSAFE_H

int safe_open(const char *pathname, int flags);
void safe_ioctl(int d, int request, void *ptr);

ssize_t safe_read(int fd, void *buf, size_t count);
ssize_t safe_write(int fd, const void *buf, size_t count);
void safe_fsync(int fd);
void safe_lseek64(int fd, long long offset, int whence);

void *safe_calloc(size_t nmemb, size_t size);

void safe_gettimeofday(struct timeval *tv);

#endif /* LIBSAFE_H */
