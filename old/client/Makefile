NAME = ft_ping

SRC_DIR =	src
OBJ_DIR = 	objs
UTILS_DIR = ../utils

SRCS =	$(SRC_DIR)/main.c
OBJS =	$(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS =	$(OBJS:.o=.d)

CC =		gcc
CFLAGS =	-Wall -Wextra -Werror -MMD -MP
INCLUDES = 	-I include\
			-I $(UTILS_DIR)/include	
UTILS_LIB =	$(UTILS_DIR)/libutils.a

all:			$(NAME)
$(NAME):		$(OBJ_DIR) $(OBJS) $(UTILS_LIB)
					$(CC) $(OBJS) $(UTILS_LIB) -o $@
$(UTILS_LIB):
					$(MAKE) -C $(UTILS_DIR)
$(OBJ_DIR):
					mkdir -p $(OBJ_DIR)
$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.c
					$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)
fclean: clean
	$(MAKE) -C $(UTILS_DIR) fclean
	rm -f $(NAME)
re:		fclean all
.PHONY: all clean fclean re