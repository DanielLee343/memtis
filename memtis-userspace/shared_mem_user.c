#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <err.h>
#include <sys/wait.h>
#define DEVICE_PATH "/dev/shared_mem_dev"
#define BUFFER_SIZE 4096
#define NUM_ADDRESSES 10
int syscall_shared_mem_start = 451;
int syscall_shared_mem_end = 452;

long shared_mem_start()
{
    return syscall(syscall_shared_mem_start);
}

long shared_mem_end()
{
    return syscall(syscall_shared_mem_end);
}

int main()
{
    shared_mem_start();
    int *buffer_size;
    unsigned long addresses_1[NUM_ADDRESSES] = {
        0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555,
        0x66666666, 0x77777777, 0x88888888, 0x99999999, 0x0};
    unsigned long addresses_2[NUM_ADDRESSES] = {
        0xAABBCCDD, 0xDDBBCCAA, 0x9988DDFF, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0};
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        shared_mem_end();
        perror("open");
        return EXIT_FAILURE;
    }

    char *shared_mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED)
    {
        shared_mem_end();
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // Write data to shared memory
    // const char *data = "Hello from user space!";
    // strcpy(shared_mem, data);

    // // Read data from shared memory
    // printf("Data read from shared memory: %s\n", shared_mem);
    buffer_size = (int *)((char *)shared_mem + BUFFER_SIZE - sizeof(int));
    // const char *data = "Hello from user space!";

    for (int i = 0; i < 6; i++)
    {
        // strcpy(shared_mem, data);
        // size_t len = strlen(shared_mem);
        // if (shared_mem[len - 1] == '\n')
        // {
        //     shared_mem[len - 1] = '\0';
        // }
        if (i % 2 == 0)
            memcpy(shared_mem, addresses_1, sizeof(addresses_1));
        else
            memcpy(shared_mem, addresses_2, sizeof(addresses_2));
        *buffer_size = 10;
        // printf("Data written to shared memory:\n");
        // for (int i = 0; i < NUM_ADDRESSES; ++i) {
        // 	printf("Address %d: 0x%lx\n", i, shared_mem[i]);
        // }
        // printf("Data written to shared memory: %s\n", shared_mem);
        sleep(1);
    }

    munmap(shared_mem, BUFFER_SIZE);
    close(fd);
    shared_mem_end();
    return EXIT_SUCCESS;
}