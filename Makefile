NAME	= ft_strace

CC		= gcc
CFLAGS	= -Wall -Wextra -Werror
IFLAGS	= -I incl

SRCS	= src/main.c

OBJS	= $(SRCS: .c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(IFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(IFLAGS) -c %< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all



.PHONY: all clean fclean re
