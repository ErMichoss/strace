#include "ft_strace.h"

void run_child(char *argv[], char *envp[])
{
    raise(SIGSTOP);
    if (execve(argv[1], &argv[1], envp) == -1) {
        FT_ERROR("execve");
        exit(1);
    }
}