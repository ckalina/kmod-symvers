CC ?= gcc
CFLAGS += -lelf

CC              ?= gcc
CFLAGS          ?= -std=c99 -D_GNU_SOURCE -lelf

CFLAGS_DEBUG    ?= -g3 -Wall -Wextra -Wpedantic

SRC             = $(wildcard *.c)
OBJ             = $(SRC:%.c=%.o)

BUILD_DIR       ?= ../bin/
BIN             ?= $(BUILD_DIR)/kmod-symvers

%.o:
	$(CC) $(CFLAGS) -c $(@:%.o=%.c) -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

build: $(BIN)
	echo Building release build ...

clean:
	rm -rf $(SRC_LEXER_C) $(SRC_PARSER_C) $(OBJ) $(BIN)

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: FFLAGS += $(FFLAGS_DEBUG)
debug: $(BIN)
	echo Building debug build ...
