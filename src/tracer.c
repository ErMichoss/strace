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

static void trace_loop(pid_t main_pid, int arch) {
    t_proc  *procs = NULL;
    t_proc  *proc;
    int     status;
    pid_t   pid;

    proc_add(&procs, proc_new(main_pid, arch));

    ptrace(PTRACE_SYSCALL, main_pid, NULL, NULL);

    while (procs) {
        pid = waitpid(-1, &status, __WALL);
        if (pid == -1) {
            if (errno == ECHILD)
                break;
            continue;
        }

        proc = proc_find(procs, pid);

        if (!proc) {
            proc = proc_new(pid, arch);
            if (!proc)
                continue;
            proc_add(&procs, proc);
            int opts = PTRACE_O_TRACESYSGOOD
                 | PTRACE_O_TRACEEXEC
                 | PTRACE_O_TRACEEXIT
                 | PTRACE_O_TRACEFORK
                 | PTRACE_O_TRACEVFORK
                 | PTRACE_O_TRACECLONE;
            ptrace(PTRACE_SETOPTIONS, pid, NULL, opts);
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            continue;
        }

        if (WIFEXITED(status))
        {
            proc_remove(&procs, pid);
            continue;
        }

        if (WIFSIGNALED(status))
        {
            if (!proc->exit_printed)
                fprintf(stderr, "+++ killed by %s +++\n", get_signal_name(WTERMSIG(status)));
            proc_remove(&procs, pid);
            continue;
        }

        if (!WIFSTOPPED(status))
        {
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            continue;
        }

        int sig = WSTOPSIG(status);

        // evento ptrace
        if (sig == SIGTRAP)
        {
            int event = (status >> 16) & 0xffff;
            if (event == PTRACE_EVENT_EXEC)
            {
                proc->entry = false;  // consumir la salida del execve
                proc->syscall_num = 59;  // forzar syscall_num a execve
            }
            if (event == PTRACE_EVENT_EXIT)
            {
                if (pid == main_pid)
                {
                    unsigned long exit_code;
                    ptrace(PTRACE_GETEVENTMSG, pid, NULL, &exit_code);
                    if (exit_code > 128)
                        fprintf(stderr, "+++ killed by %s +++\n", get_signal_name(exit_code - 128));
                    else
                        fprintf(stderr, "+++ exited with %lu +++\n", exit_code);
                    proc->exit_printed = true;  // ← marcar como impreso
                }
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            continue;
        }

        // señal real
        if (sig != (SIGTRAP | 0x80))
        {
            if (sig == SIGSTOP)
            {
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

        // syscall stop
        t_regs regs;
        if (get_registers(pid, &regs) == -1)
        {
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            continue;
        }

        int p_arch = regs.is_32bit ? ARCH_32 : ARCH_64;
        proc->arch = p_arch;

        if (proc->entry)
        {
            proc->syscall_num = regs.is_32bit ? regs.reg32.orig_eax : regs.reg64.orig_rax;

            t_syscall *table = regs.is_32bit ? g_syscall_table_32 : g_syscall_table_64;
            int max = regs.is_32bit ? MAX_SYSCALLS_32 : MAX_SYSCALLS_64;
            t_syscall *syscall = NULL;

            if (proc->syscall_num >= 0 && proc->syscall_num < max)
                syscall = &table[proc->syscall_num];

            print_syscall_entry(pid, proc->syscall_num, syscall, &regs, p_arch);

            if (proc->syscall_num == 60 || proc->syscall_num == 231)
            {
                fprintf(stderr, ") = ?\n");
                proc->entry = false;
                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
                continue;
            }
            proc->entry = false;
        }
        else
        {
            print_syscall_exit(&regs, proc->arch, proc->syscall_num);
            proc->entry = true;
        }

        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }

    proc_free_all(&procs);
}

int run_tracer(pid_t pid, int arch)
{
    int status;
    int opts;

    opts = PTRACE_O_TRACESYSGOOD
         | PTRACE_O_TRACEEXEC
         | PTRACE_O_TRACEEXIT
         | PTRACE_O_TRACEFORK
         | PTRACE_O_TRACEVFORK
         | PTRACE_O_TRACECLONE;

    if (ptrace(PTRACE_SEIZE, pid, NULL, opts) == -1) {
        FT_ERROR("PTRACE_SEIZE");
        return 1;
    }
    if (waitpid(pid, &status, 0) == -1) {
        FT_ERROR("waitpid");
        return 1;
    }

    trace_loop(pid, arch);
    return 0;
}