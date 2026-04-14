#ifndef FT_STRACE_H
# define FT_STRACE_H

# include <string.h>
# include <strings.h>
# include <ctype.h>
# include <unistd.h>
# include <pthread.h>
# include <errno.h>
# include <signal.h>
# include <fcntl.h>
# include <elf.h>
# include <sys/types.h>
# include <sys/ptrace.h>
# include <sys/reg.h>
# include <sys/wait.h>
# include <sys/user.h>
# include <sys/uio.h>
# include <sys/queue.h>
# include <sys/utsname.h>
# include <sys/stat.h>
# include <stdbool.h>

# ifndef DEBUG
#  define DEBUG 0
# endif

#define FT_ERROR(context) ft_error(context, __FILE__, __LINE__)


// ___ AUX ___
size_t	ft_strlen(const char *str);

// ___ MAIN ___
int main(int argc, char *argv[], char *envp[]);

// ___ CHILD ___


#endif
