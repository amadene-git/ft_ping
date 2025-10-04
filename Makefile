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

# Nom du compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -Wextra -Iinclude

# Répertoires
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Fichiers source et objets
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Nom de l'exécutable
TARGET = my_project

# Règle principale
all: $(OBJ_DIR) $(TARGET)

# Lien final
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compilation des .c en .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -I ${INC_DIR} -o $@

# Création du dossier obj
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Nettoyage
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean