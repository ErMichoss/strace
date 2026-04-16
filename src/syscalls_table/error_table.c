#include "ft_strace.h"

static t_error g_errors[] = {
    {1,   "EPERM"},
    {2,   "ENOENT"},
    {3,   "ESRCH"},
    {4,   "EINTR"},
    {5,   "EIO"},
    {6,   "ENXIO"},
    {7,   "E2BIG"},
    {8,   "ENOEXEC"},
    {9,   "EBADF"},
    {10,  "ECHILD"},
    {11,  "EAGAIN"},
    {12,  "ENOMEM"},
    {13,  "EACCES"},
    {14,  "EFAULT"},
    {16,  "EBUSY"},
    {17,  "EEXIST"},
    {19,  "ENODEV"},
    {20,  "ENOTDIR"},
    {21,  "EISDIR"},
    {22,  "EINVAL"},
    {24,  "EMFILE"},
    {25,  "ENOTTY"},
    {28,  "ENOSPC"},
    {32,  "EPIPE"},
    {38,  "ENOSYS"},
    {0,   NULL}
};

char    *get_error_name(long errnum)
{
    int i;

    errnum = -errnum;
    i = 0;
    while (g_errors[i].name != NULL)
    {
        if (g_errors[i].num == errnum)
            return g_errors[i].name;
        i++;
    }
    return "EUNKNOWN";
}