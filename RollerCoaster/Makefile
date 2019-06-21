CC=gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -pthread
NAME=proj2

$(NAME): $(NAME).c $(NAME).h 
	$(CC) $(CFLAGS) $(NAME).c -o $(NAME)

cleanall: clean
	rm -f $(NAME).c $(NAME).h 

clean: 
	rm -f $(NAME).o $(NAME)
