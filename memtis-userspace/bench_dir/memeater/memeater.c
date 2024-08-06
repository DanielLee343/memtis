#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ONE_GB ((size_t)1024 * 1024 * 1024)
#define TOTAL_MEM (16 * ONE_GB)

int main()
{
    void *mem = malloc(TOTAL_MEM);
    if (mem == NULL)
    {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < 16; i++)
    {
        memset((char *)mem + (i * ONE_GB), 0, ONE_GB);
        printf("memset %zu GB of memory\n", i + 1);
        sleep(1);
    }
    printf("clearning up\n");
    sleep(1);

    free(mem);

    return 0;
}