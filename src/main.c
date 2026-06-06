#include "ft_strace.h"

int main(int argc, char *argv[], char *envp[])
{
    int     arch;
    char    *binary;

    if (argc < 2) {
        char *err = "ft_strace: must have PROG [ARGS]\n";
        write(2, err, ft_strlen(err));
        return 1;
    }

    // encontrar el binario
    if (argv[1][0] == '/' || (argv[1][0] == '.' && argv[1][1] == '/'))
        binary = argv[1];
    else
        binary = ft_find_binary(argv[1], envp);

    if (!binary) {
        write(2, "ft_strace: ", 11);
        write(2, argv[1], ft_strlen(argv[1]));
        write(2, ": command not found\n", 20);
        return 1;
    }

    // detectar arquitectura
    arch = detect_arch(binary);

    pid_t pid = fork();
    if (pid == -1) {
        FT_ERROR("fork");
        return 1;
    }
    if (pid == 0) {
        run_child(argv, envp);
    } else {
        run_tracer(pid, arch);
    }

    return 0;
}
