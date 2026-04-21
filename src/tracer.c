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
            if (errno == ESRCH) {
                // esperar a que el proceso esté listo
                if (waitpid(pid, &status, WNOHANG) == 0)
                    continue;   // proceso sigue vivo, reintentar
                break;          // proceso realmente terminó
            }
            FT_ERROR("ptrace(PTRACE_SYSCALL)");
            return ;
        }
        if (waitpid(pid, &status, 0) == -1) {
            FT_ERROR("waitpid");
            return ;
        }

        if (WIFEXITED(status)) {
            fprintf(stderr, "+++ exited with %d +++\n", WEXITSTATUS(status));
            break;
        }

        if (WIFSIGNALED(status)) {
            fprintf(stderr, "+++ killed by %s +++\n", get_signal_name(WTERMSIG(status)));
            break;
        }

        if (WIFSTOPPED(status) && (WSTOPSIG(status) != (SIGTRAP | 0x80)))
        {
            int sig = WSTOPSIG(status);

            if (sig == SIGTRAP)
            {
                // comprobar si es evento EXEC
                int event = (status >> 16) & 0xffff;
                if (event == PTRACE_EVENT_EXEC)
                {
                    fprintf(stderr, ") = 0\n");
                    entry = true;
                    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
                    continue;
                }
                if (event == PTRACE_EVENT_EXIT)
                {
                    unsigned long exit_code;
                    ptrace(PTRACE_GETEVENTMSG, pid, NULL, &exit_code);
                    fprintf(stderr, "+++ exited with %lu +++\n", exit_code);
                    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
                    continue;
                }
                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
                continue;
            }

            siginfo_t siginfo;
            if (ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo) == 0)
                fprintf(stderr, "--- %s {si_signo=%s, si_code=%d} ---\n",
                        get_signal_name(sig), get_signal_name(sig), siginfo.si_code);
            else
                fprintf(stderr, "--- %s ---\n", get_signal_name(sig));

            ptrace(PTRACE_SYSCALL, pid, NULL, (void *)(long)sig);
            continue;
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

                if (syscall_num == 60 || syscall_num == 231)
                {
                    fprintf(stderr, ") = ?\n");
                    entry = false;
                    continue;
                }

                entry = false;
            } else {
                t_syscall *table = regs.is_32bit ? g_syscall_table_32 : g_syscall_table_64;
                int max = regs.is_32bit ? MAX_SYSCALLS_32 : MAX_SYSCALLS_64;
                if (syscall_num >= 0 && syscall_num < max)
                    syscall = &table[syscall_num];
                else 
                    syscall = NULL;
                print_syscall_exit(&regs, arch, syscall->name);
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