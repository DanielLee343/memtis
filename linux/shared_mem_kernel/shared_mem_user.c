#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define DEVICE "/dev/shared_mem_device"
#define BUFFER_SIZE 4096  // Ensure this matches the size used in the kernel

int main() {
    int fd;
    char *shared_mem;
    int *buffer_size;

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    shared_mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("Failed to mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    buffer_size = (int *)(shared_mem + BUFFER_SIZE - sizeof(int));

    while (1) {
        printf("Enter data to send to kernel: ");
        if (fgets(shared_mem, BUFFER_SIZE - sizeof(int), stdin) == NULL) {
            perror("Failed to read input");
            munmap(shared_mem, BUFFER_SIZE);
            close(fd);
            return EXIT_FAILURE;
        }

        size_t len = strlen(shared_mem);
        if (shared_mem[len - 1] == '\n') {
            shared_mem[len - 1] = '\0';
        }

        // Notify the kernel thread by updating buffer_size
        *buffer_size = len;

        printf("Data sent to kernel: %s\n", shared_mem);
    }

    munmap(shared_mem, BUFFER_SIZE);
    close(fd);
    return EXIT_SUCCESS;
}