#include "ft_strace.h"

char *get_signal_name(int signum) 
{
    int i = 0;
    while (g_signals[i].name != NULL) {
        if (g_signals[i].num == signum) {
            return g_signals[i].name;
        }
        i++;
    }
    return "UNKNOWN";
}