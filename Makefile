# Makefile for Dogecoin Stratum Miner (scrypt) on Raspberry Pi Zero 2W

CC = gcc
CFLAGS = -O2 -std=c99 -Wall -I.       # ������ Добавлено -I. для config.h в корне
LDFLAGS = -ljansson -lssl -lcrypto

SRC_DIR = src
OBJ_DIR = obj
BIN = doge-miner

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN)
