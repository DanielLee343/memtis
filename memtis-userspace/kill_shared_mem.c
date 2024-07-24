#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <err.h>
#include <sys/wait.h>
int syscall_shared_mem_end = 452;
long shared_mem_end()
{
    return syscall(syscall_shared_mem_end);
}

int main()
{
    shared_mem_end();
}