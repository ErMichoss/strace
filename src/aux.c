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

char    *get_error_name(long errnum)
{
    (void)errnum;
    return ("EUNKNOWN");
}