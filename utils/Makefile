# === Makefile du dossier utils ===

CC = gcc
CFLAGS = -Wall -Wextra -Werror

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include
LIB_DIR = ../lib
LIB_NAME = libutils.a

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
LIB = $(LIB_DIR)/$(LIB_NAME)

all: $(LIB)

# Création de la lib statique
$(LIB): $(OBJS)
	mkdir -p $(LIB_DIR)
	ar rcs $@ $^

# Compilation des .c en .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(INC_DIR) -c $< -o $@


clean:
					rm -rf $(OBJ_DIR)
fclean:			clean
					rm -rf $(LIB)
re:				fclean all

.PHONY: all clean fclean re $(LIB)