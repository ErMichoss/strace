#include "ft_strace.h"

static void trace_loop(pid_t pid)
{
    bool entry = true;
    int syscall_num = 0;
    int status;

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
            if (entry) {
                //leer syscall number
                // Imprimir entrada
                entry = false;
            } else {
                //Imprimir salida
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