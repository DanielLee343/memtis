#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

int syscall_htmm_start = 449;
int syscall_htmm_end = 450;
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

int main(int argc, char **argv)
{
    pid_t pid;
    int state;

    if (argc < 2)
    {
        printf("Usage ./launch_bench [BENCHMARK]");
        shared_mem_end();
        return 0;
    }

    pid = fork();
    if (pid == 0)
    {
        execvp(argv[1], &argv[1]);
        perror("Fails to run bench");
        exit(-1);
    }
    // #ifdef __NOPID
    //     htmm_start(-1, 0);
    // #else
    shared_mem_start();
    // #endif
    printf("pid: %d\n", pid);
    waitpid(pid, &state, 0);

    shared_mem_end();

    return 0;
}
