CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -I./include
NAME = webserv

SRC = $(shell find ./src -name '*.cpp')

OBJ_DIR = obj/
OBJ = $(patsubst ./src/%.cpp, $(OBJ_DIR)%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)%.o: ./src/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

v:
	valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./$(NAME) webserv.conf

re: fclean all
