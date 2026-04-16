NAME	= ft_strace

CC		= gcc
CFLAGS	= -Wall -Wextra -Werror
IFLAGS	= -I incl

SRCS	=	src/main.c \
			src/tracer.c \
			src/aux.c \
			src/child.c \
			src/output.c \
			src/buffer.c \
			src/signal.c \
			src/syscalls_table/table_64.c \
			src/syscalls_table/table_32.c \
			src/syscalls_table/error_table.c \
			src/syscalls_table/signal_table.c \
			src/libft/ft_split.c \
			src/libft/ft_strjoin.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(IFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

debug: CFLAGS += -g -DDEBUG=1
debug: re

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all



.PHONY: all clean fclean re debug
