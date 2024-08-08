#include <stdio.h>
#include <stdlib.h>
// #include <omp.h>

#define ARRAY_SIZE 100000000
#include "shared_header.h"

extern void populate_shared_mem(void *addr, int size);

int launch_bench();
int main(int argc, char **argv)
{
    int state;
    int fd;

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    shared_mem = (unsigned long *)mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }
    buffer_index = (int *)((char *)shared_mem + BUFFER_SIZE - sizeof(int));
    buffer_is_ready = (bool *)((char *)shared_mem + BUFFER_SIZE - sizeof(int) - sizeof(bool));
    *buffer_index = 0;
    *buffer_is_ready = false;

    int pid = getpid();
    printf("parent pid: %d\n", pid);
    shared_mem_start(pid);
    // execvp(argv[1], &argv[1]);
    launch_bench(); // benchmark entry

    shared_mem_end();
    munmap(shared_mem, BUFFER_SIZE); // Unmap before exiting
    close(fd);
    return 0;
}

int launch_bench()
{

    int *array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (array == NULL)
    {
        perror("Unable to allocate memory");
        return EXIT_FAILURE;
    }

// Initialize the array with zeros in parallel
#pragma omp parallel for
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        array[i] = 0;
    }

// Modify the array in parallel
#pragma omp parallel for
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
#pragma omp atomic
        array[i] += i;
    }

    // Verify the results
    int correct = 1;
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (array[i] != i)
        {
            correct = 0;
            break;
        }
    }

    if (correct)
    {
        printf("All elements are correctly modified.\n");
    }
    else
    {
        printf("There was an error in the modification of the elements.\n");
    }

    // Free the allocated memory
    free(array);

    return 0;
}