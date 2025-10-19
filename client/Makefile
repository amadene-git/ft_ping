# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: admadene <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/04 09:09:42 by admadene          #+#    #+#              #
#    Updated: 2025/10/04 09:09:44 by admadene         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC = gcc
CFLAGS = -Wall -Wextra -Werror

# Répertoires
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

#Fichiers
SRCS =	$(SRC_DIR)/main.c
OBJS =	$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))



NAME = ft_ping

all:			$(OBJ_DIR) $(NAME)

$(OBJ_DIR):
					mkdir -p $(OBJ_DIR)


$(NAME):		$(OBJS)
					$(CC) $(OBJS) -o $@

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.c
					$(CC) $(CFLAGS) -c $< -I ${INC_DIR} -o $@



clean:
					rm -rf $(OBJ_DIR)
fclean:			clean
					rm -rf $(NAME)
re:				fclean all

.PHONY: all clean fclean re $(NAME)