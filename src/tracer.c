#include "ft_strace.h"

static int get_registers(pid_t pid, t_regs *regs)
{
    struct iovec    iov;
    iov.iov_base = regs;
    iov.iov_len = sizeof(*regs);

    if (ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov) == -1) {
        FT_ERROR("ptrace(PTRACE_GETREGSET)");
        return -1;
    }

    regs->is_32bit = (iov.iov_len == sizeof(t_regs32));
    return 0;
}

static void trace_loop(pid_t pid)
{
    bool        entry = true;
    int         syscall_num = 0;
    t_syscall   *syscall = NULL;
    int         status;
    t_regs      regs;

    while (1) {
        if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1) {
            FT_ERROR("ptrace(PTRACE_SYSCALL)");
            return ;
        }
        if (waitpid(pid, &status, 0) == -1) {
            FT_ERROR("waitpid");
            return ;
        }

        if (WIFEXITED(status)) {
            //Imprimir exit
            break;
        }

        if (WIFSIGNALED(status)) {
            //Imprimir Señal
            break;
        }

        if (WIFSTOPPED(status) && (WSTOPSIG(status) == (SIGTRAP | 0x80))) {
            if (get_registers(pid, &regs) == -1)
                return;

            int arch = regs.is_32bit ? ARCH_32 : ARCH_64;
            if (entry) {
                if(regs.is_32bit)
                    syscall_num = regs.reg32.orig_eax;
                else
                    syscall_num = regs.reg64.orig_rax;
                
                t_syscall *table = regs.is_32bit ? g_syscall_table_32 : g_syscall_table_64;
                int max = regs.is_32bit ? MAX_SYSCALLS_32 : MAX_SYSCALLS_64;

                if (syscall_num >= 0 && syscall_num < max)
                    syscall = &table[syscall_num];
                else 
                    syscall = NULL;
                print_syscall_entry(pid, syscall_num, syscall, &regs, arch);
                entry = false;
            } else {
                print_syscall_exit(&regs, arch);
                entry = true;
            }
        }
    }
}

int run_tracer(pid_t pid)
{
    int status;
    int opts;

    opts = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT;
    if (ptrace(PTRACE_SEIZE, pid, NULL, opts) == -1){
        FT_ERROR("PTRACE_SEIZE");
        return 1;
    }
    
    if (waitpid(pid, &status, 0) == -1) {
        FT_ERROR("waitpid");
        return 1;
    }

    trace_loop(pid);
    return 0;
}