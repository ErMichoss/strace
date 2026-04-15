#ifndef FT_STRACE_H
# define FT_STRACE_H

# include <string.h>
# include <strings.h>
# include <ctype.h>
# include <unistd.h>
# include <stdlib.h>
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
# include <stdio.h> 

# ifndef DEBUG
#  define DEBUG 0
# endif

# define MAX_SYSCALLS_64 462
# define MAX_SYSCALLS_32 387

# define ARCH_32 1
# define ARCH_64 2

#define FT_ERROR(context) ft_error(context, __FILE__, __LINE__)


typedef struct s_regs32
{
    uint32_t    ebx;
    uint32_t    ecx;
    uint32_t    edx;
    uint32_t    esi;
    uint32_t    edi;
    uint32_t    ebp;
    uint32_t    eax;
    uint32_t    xds;
    uint32_t    xes;
    uint32_t    xfs;
    uint32_t    xgs;
    uint32_t    orig_eax;
    uint32_t    eip;
    uint32_t    xcs;
    uint32_t    eflags;
    uint32_t    esp;
    uint32_t    xss;
}   t_regs32;

typedef struct s_regs
{
    union {
        struct user_regs_struct reg64;
        t_regs32                reg32;
    };
    bool    is_32bit;
}   t_regs;

typedef enum
{
    VOID,
    INT,
    STRING,
    POINTER,
}   e_arg_type;



typedef struct s_syscall
{
    char        *name;
    e_arg_type  arg_types[6];
}   t_syscall;

extern t_syscall g_syscall_table_64[];
extern t_syscall g_syscall_table_32[];

// ___ AUX ___
size_t	ft_strlen(const char *str);
void	ft_error(const char *context, const char *file, int line);
char    *get_error_name(long errnum); //TODO temporal
void    free_str_matrix(char **mtx);
char	*ft_strjoin(char const *s1, char const *s2);
char	**ft_split(char const *s, char c);

// ___ MAIN ___
int main(int argc, char *argv[], char *envp[]);

// ___ CHILD ___
void run_child(char *argv[], char *envp[]);

// ___ TRACER ___
int run_tracer(pid_t pid);

// ___ OUTPUT ___
void print_syscall_entry(pid_t pid, int syscall_num, t_syscall *syscall, t_regs *regs, int arch);
void print_syscall_exit(t_regs *regs, int arch, int syscall_num);


#endif
