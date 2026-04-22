#include "ft_strace.h"

static char *ft_find_binary(char *arg, char *envp[])
{
    char *path = NULL;

    for (int i = 0; envp[i] != NULL; i++) {
        if (strncmp("PATH=", envp[i], 5) == 0) {
            path = strdup(envp[i] + 5);
        }           
    }

    if (!path)
        return NULL;

    char **dirs = ft_split(path, ':');
    free(path);

    char *candidate;
    for (int i = 0; dirs[i] != NULL; i++) {
        char *aux = ft_strjoin(dirs[i], "/");
        candidate = ft_strjoin(aux, arg);
        if (access(candidate, X_OK) == 0) {
            free_str_matrix(dirs);
            free(aux);
            return candidate;
        }
        free(aux);
        free(candidate);
    }

    free_str_matrix(dirs);
    return NULL;
}

void run_child(char *argv[], char *envp[])
{
    raise(SIGSTOP);
    if (argv[1][0] == '/' || (argv[1][0] == '.' && argv[1][1] == '/')) {
        if (execve(argv[1], &argv[1], envp) == -1) {
            FT_ERROR("execve");
            exit(1);
        }
    } else {
        char *bin = ft_find_binary(argv[1], envp);
        if(bin == NULL) {
            write(2, "ft_strace: ", 11);
            write(2, argv[1], ft_strlen(argv[1]));
            write(2, ": command not found\n", 20);
            exit(1);
        }
        if (execve(bin, &argv[1], envp) == -1) {
            free(bin);
            FT_ERROR("execve");
            exit(1);
        }
    }
}