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
    buffer_size = (int *)(shared_mem + BUFFER_SIZE - sizeof(int));
    const char *data = "Hello from user space!";
    for (int i = 0; i < 6; i++)
    {
        strcpy(shared_mem, data);
        size_t len = strlen(shared_mem);
        if (shared_mem[len - 1] == '\n')
        {
            shared_mem[len - 1] = '\0';
        }
// 、、ioctl
        // Notify the kernel thread by updating buffer_size
        *buffer_size = len;
        printf("Data written to shared memory: %s\n", shared_mem);
        sleep(1);
    }

    munmap(shared_mem, BUFFER_SIZE);
    close(fd);
    shared_mem_end();
    return EXIT_SUCCESS;
}