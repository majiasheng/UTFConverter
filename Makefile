CC = gcc
CFLAGS = -Wall -Werror -pedantic -Wextra -std=gnu89 -g
BIN_DIR = ./bin
EXE = utf
BIN = $(BIN_DIR)/$(EXE)
BUILD_DIR = ./build
INCLUDE = $(wildcard include/)
SRC_DIR = ./src
SRC = $(wildcard src/*.c)
REQ = $(SRC) $(wildcard include/*.h)


all: $(BIN)

$(BIN): $(BUILD_DIR)/utfconverter.o 
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(wildcard include/*.h)
	mkdir -p $(BIN_DIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -fR $(BIN_DIR) $(BUILD_DIR)

# debug:
# 	$(BIN): $(REQ)
# 	$(CC) -g $(CFLAGS) -I $(INCLUDE) $(SRC) -o $(BIN)
