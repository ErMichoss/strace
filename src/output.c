#include "ft_strace.h"

void print_syscall_entry(pid_t pid, int syscall_num, t_syscall *syscall, t_regs *regs, int arch)
{
    long    args[6];
    int     i;

    (void)pid;

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
        else if (syscall->arg_types[i] == STRING)
            // leer string de memoria del hijo
            printf("%p", (void *)args[i]);   // temporal
        i++;
    }
}

void print_syscall_exit(t_regs *regs, int arch)
{
    long    ret;

    (void)regs;   // silenciar warning temporalmente
    (void)arch;

    ret = arch == ARCH_32 ? regs->reg32.eax : regs->reg64.rax;

    if (ret < 0) {
        //mostrar código de error
        printf(") = -1 %s (%s)\n", get_error_name(ret), strerror(-ret));
    }
    else {
        printf(") = %d\n", (int)ret);
    }
}