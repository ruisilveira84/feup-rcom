SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/main
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

CC			:=	gcc
CFLAGS		:=	-Wall
CPPFLAGS	:=	-Iinclude
LDLIBS		:=	-lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

debug: CFLAGS := $(CFLAGS) -DDEBUG
debug: clean $(EXE)

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR)