#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <err.h>
#include <time.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

__thread int *buffer_index;
// int *buffer_index;

#define DEVICE_PATH "/dev/my_char_device"
#define BUFFER_SIZE 4096
#define NUM_SLOTS (BUFFER_SIZE / 8)
#define NUM_ADDRESSES 10 // for testing
#define COPIES 64

int syscall_shared_mem_start = 451;
int syscall_shared_mem_end = 452;
unsigned long *shared_mem;
bool *buffer_is_ready;
static inline long shared_mem_start(pid_t pid)
{
    return syscall(syscall_shared_mem_start, pid);
}

static inline long shared_mem_end()
{
    return syscall(syscall_shared_mem_end);
}

#endif // SHARED_H