#include "ft_strace.h"

int main(int argc, char *argv[], char *envp[])
{
	if (argc < 2) {
		char *err = "ft_strace: must have PROG [ARGS]\n";
		write(2, err, ft_strlen(err));
		return 1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		FT_ERROR("fork");
		return 1;
	}
	if (pid == 0) {
		run_child(argv, envp);
	} else {
		run_tracer(pid);
	}

	return 0;
}
