NAME = libutils.a

SRC_DIR = src
OBJ_DIR = objs
INC_DIR = include

SRCS =	$(SRC_DIR)/rawSocket.c \
		$(SRC_DIR)/utils.c

OBJS =	$(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS =	$(OBJS:.o=.d)

CC = gcc
CFLAGS = -Wall -Wextra -Werror -MMD -MP
INCLUDES = -I $(INC_DIR)


all: $(NAME)

$(NAME): $(OBJ_DIR) $(OBJS)
	ar rcs $@ $(OBJS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)


clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re