#include "ft_strace.h"

void read_buffer(pid_t pid, unsigned long addr, size_t len)
{
    char    path[64];
    char    buffer[32];
    int     fd;
    ssize_t n;
    size_t  to_read;

    to_read = len < 32 ? len : 32;

    snprintf(path, sizeof(path), "/proc/%d/mem", pid);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "%p", (void *)addr);
        return ;
    }
    lseek(fd, (off_t)addr, SEEK_SET);
    n = read(fd, buffer, to_read);
    close(fd);

    if (n <= 0) {
        fprintf(stderr, "%p", (void *)addr);
        return ;
    }

    fprintf(stderr, "\"");
    for (ssize_t i = 0; i < n; i++) {
        if (buffer[i] == '\n')
            fprintf(stderr, "\\n");
        else if (buffer[i] == '\t')
            fprintf(stderr, "\\t");
        else if (buffer[i] >= 32 && buffer[i] <= 126)
            fprintf(stderr, "%c", buffer[i]);
        else
            fprintf(stderr, "\\%03o", (unsigned char)buffer[i]);
    }
    fprintf(stderr, "\"");

    if (len > 32)
        fprintf(stderr, "...");
}