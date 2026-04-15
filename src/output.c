#include "ft_strace.h"

/**
 * @brief   Lee un string de la memoria del proceso hijo
 *
 * @description
 *   El proceso hijo tiene sus propias strings en su espacio de memoria.
 *   El padre solo recibe la dirección donde están. Para leerlas, usamos
 *   /proc/<pid>/mem que representa toda la memoria del proceso hijo.
 *   Saltamos a la dirección con lseek y leemos hasta encontrar un '\0'.
 *
 * @param   pid     PID del proceso hijo
 * @param   addr    Dirección de memoria donde está el string en el hijo
 * @param   buffer  Buffer donde se guardará el string leído
 * @param   max_len Tamaño máximo del buffer
 *
 * @return  void - El resultado se escribe en buffer.
 *          Si falla, buffer queda como string vacío.
 */
static void read_string(pid_t pid, unsigned long addr, char *buffer, size_t max_len)
{
    char    path[64];
    int     fd;

    if (addr == 0) {
        buffer[0] = '\0';
        return ;
    }

    snprintf(path, sizeof(path), "/proc/%d/mem", pid);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        buffer[0] = '\0';
        return ;
    }

    lseek(fd, (off_t)addr, SEEK_SET);
    ssize_t n = read(fd, buffer, max_len - 1);
    close(fd);

    if (n <= 0) {
        buffer[0] = '\0';
        return ;
    }

    // truncar en el primer null
    buffer[n] = '\0';
    for (size_t i = 0; i < (size_t)n; i++) {
        if (buffer[i] == '\0')
            return ;
    }
    buffer[max_len - 1] = '\0';
}

void print_syscall_entry(pid_t pid, int syscall_num, t_syscall *syscall, t_regs *regs, int arch)
{
    long    args[6];
    int     i;

    // Recoger argumentos según arquitectura
    if (arch == ARCH_32)
    {
        args[0] = regs->reg32.ebx;
        args[1] = regs->reg32.ecx;
        args[2] = regs->reg32.edx;
        args[3] = regs->reg32.esi;
        args[4] = regs->reg32.edi;
        args[5] = regs->reg32.ebp;
    }
    else
    {
        args[0] = regs->reg64.rdi;
        args[1] = regs->reg64.rsi;
        args[2] = regs->reg64.rdx;
        args[3] = regs->reg64.r10;
        args[4] = regs->reg64.r8;
        args[5] = regs->reg64.r9;
    }

    // Nombre de la syscall
    if (!syscall || !syscall->name) {
        printf("syscall_%d(", syscall_num);
        return ;
    } else {
        printf("%s(", syscall->name);
    }

    // Argumentos
    i = 0;
    while (i < 6 && syscall->arg_types[i] != VOID)
    {
        if (i > 0)
            printf(", ");

        if (syscall->arg_types[i] == INT)
            printf("%d", (int)args[i]);
        else if (syscall->arg_types[i] == POINTER)
            printf("%p", (void *)args[i]);
        else if (syscall->arg_types[i] == STRING) {
            char tmp[256];
            read_string(pid, args[i], tmp, sizeof(tmp));
            printf("\"%s\"", tmp);
        }
        i++;
    }
}

void print_syscall_exit(t_regs *regs, int arch, int syscall_num)
{
    long    ret;

    ret = arch == ARCH_32 ? regs->reg32.eax : regs->reg64.rax;

    if (ret < 0)
        printf(") = -1 %s (%s)\n", get_error_name(ret), strerror(-ret));
    else if (syscall_num == 9  ||   // mmap
             syscall_num == 12 ||   // brk
             syscall_num == 25)     // mremap
        printf(") = %p\n", (void *)ret);
    else
        printf(") = %d\n", (int)ret);
}