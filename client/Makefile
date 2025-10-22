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
LIB_DIR = ../lib
INCLUDES = -Iinclude -I../utils/include

# Fichiers
SRCS =	$(SRC_DIR)/ft_ping.c
OBJS =	$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
LIBS =	$(LIB_DIR)/libutils.a



NAME = 			ft_ping

all:			$(LIBS) $(OBJ_DIR) $(NAME)

$(LIBS):
					$(MAKE) -C ../utils 

$(OBJ_DIR):
					mkdir -p $(OBJ_DIR)


$(NAME):		$(OBJS)
					$(CC) $(OBJS) -L$(LIB_DIR) -lutils -o $@

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.c
					$(CC) $(CFLAGS) -c $< ${INCLUDES} -o $@



clean:
					rm -rf $(OBJ_DIR)
fclean:			clean
					rm -rf $(NAME)
re:				fclean all

.PHONY: all clean fclean re