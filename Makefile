NAME	= ft_ping

SRC_DIR	= src
OBJ_DIR	= objs

SRCS	= $(SRC_DIR)/main.c\
		  $(SRC_DIR)/netUtils.c\
		  $(SRC_DIR)/timeUtils.c\
		  $(SRC_DIR)/list.c\
		  $(SRC_DIR)/stats.c\
		  $(SRC_DIR)/utils.c
OBJS	= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS	= $(OBJS:.o=.d)

CC 			= gcc
CFLAGS 		= -Wall -Wextra -Werror -MMD -MP -g
INCLUDES 	= -I include

all				: $(NAME)
$(NAME)			: $(OBJ_DIR) $(OBJS)
					$(CC) $(OBJS) -o $@
$(OBJ_DIR)		:
					mkdir -p $(OBJ_DIR)
$(OBJ_DIR)/%.o	: $(SRC_DIR)/%.c
					$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
-include $(DEPS)

clean	:
			rm -rf $(OBJ_DIR)
fclean	: clean
			rm -f $(NAME)
re		: fclean all
.PHONY	: all clean fclean re