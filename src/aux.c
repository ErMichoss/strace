#include "ft_strace.h"

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (str[i] != '\0')
		i++;
	return (i);
}

void	ft_error(const char *context, const char *file, int line)
{
    char	*err;

	err = strerror(errno);
	write(2, "ft_strace: ", 11);
	write(2, context, ft_strlen(context));
	write(2, ": ", 2);
	write(2, err, ft_strlen(err));
	write(2, "\n", 1);
	if (DEBUG)
	{
		printf("  file: %s - line: %i\n", file, line);
	}
}

void free_str_matrix(char **mtx)
{
    for (int i = 0; mtx[i]; i++)
        free(mtx[i]);
    free(mtx);
}

t_proc  *proc_new(pid_t pid, int arch)
{
    t_proc *proc;

    proc = malloc(sizeof(t_proc));
    if (!proc)
        return NULL;
    proc->pid        = pid;
    proc->entry      = true;
    proc->syscall_num = 0;
    proc->arch       = arch;
    proc->next       = NULL;
    return proc;
}

t_proc  *proc_find(t_proc *list, pid_t pid)
{
    while (list)
    {
        if (list->pid == pid)
            return list;
        list = list->next;
    }
    return NULL;
}

void    proc_add(t_proc **list, t_proc *proc)
{
    proc->next = *list;
    *list = proc;
}

void    proc_remove(t_proc **list, pid_t pid)
{
    t_proc *curr = *list;
    t_proc *prev = NULL;

    while (curr)
    {
        if (curr->pid == pid)
        {
            if (prev)
                prev->next = curr->next;
            else
                *list = curr->next;
            free(curr);
            return ;
        }
        prev = curr;
        curr = curr->next;
    }
}

void    proc_free_all(t_proc **list)
{
    t_proc *curr = *list;
    t_proc *next;

    while (curr)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    *list = NULL;
}