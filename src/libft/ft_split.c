#include "ft_strace.h"

static char	**ft_empty_split(void)
{
	char	**aux;

	aux = (char **)malloc(sizeof(char *));
	if (!aux)
		return (NULL);
	aux[0] = NULL;
	return (aux);
}

static char	*ft_strdup_custom(const char *s, size_t n)
{
	char	*dst;
	size_t	i;

	i = 0;
	if (n == 0)
		return (NULL);
	dst = (char *)malloc(sizeof(char) * (n + 1));
	if (dst == 0)
		return (NULL);
	while (i < n)
	{
		dst[i] = s[i];
		i++;
	}
	dst[i] = '\0';
	return (dst);
}

static char	**ft_free_str(char **aux)
{
	int	i;

	i = 0;
	while (aux[i])
	{
		free(aux[i]);
		i++;
	}
	free(aux);
	return (NULL);
}

static int	ft_countc(char const *s, char c)
{
	int	i;
	int	x;

	i = 0;
	x = 0;
	while (s[i] != '\0')
	{
		if (s[i] == c)
			i++;
		if (s[i] == '\0')
			return (x);
		if (s[i] != c && (s[i + 1] == c || s[i + 1] == '\0'))
			x++;
		i++;
	}
	return (x);
}

char	**ft_split(char const *s, char c)
{
	char	**aux;
	int		i;
	int		j;
	int		memo;

	i = 0;
	j = 0;
	if (!s || *s == '\0')
		return (ft_empty_split());
	aux = (char **)malloc(sizeof(char *) * (ft_countc(s, c) + 1));
	if (!aux)
		return (NULL);
	while (j < ft_countc(s, c) && s[i] != '\0')
	{
		while (s[i] == c)
			i++;
		memo = i;
		while (s[i] != c && s[i] != '\0')
			i++;
		aux[j] = ft_strdup_custom(&s[memo], i - memo);
		if (aux[j++] == NULL)
			return (ft_free_str(aux));
	}
	aux[j] = NULL;
	return (aux);
}