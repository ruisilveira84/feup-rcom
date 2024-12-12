SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/download
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

CC			:=	gcc
CFLAGS		:=	-g -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wunreachable-code -fsanitize=address
CPPFLAGS	:=	-Iinclude
LDFLAGS		:=	-fsanitize=address

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR) *.txt