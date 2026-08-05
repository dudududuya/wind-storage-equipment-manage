CC = gcc
CFLAGS = -Wall -g -O2
INC = -I ./inc

SRC_COMMON = src/dev_operate.c src/hash_table.c src/tree_print.c src/file_io.c src/protocol.c
SRC_ENGINE = src/engine.c $(SRC_COMMON)
SRC_CLIENT = src/client.c $(SRC_COMMON)

TARGET_ENGINE = engine
TARGET_CLIENT = client

all: $(TARGET_ENGINE) $(TARGET_CLIENT)

$(TARGET_ENGINE): $(SRC_ENGINE)
	$(CC) $(CFLAGS) $(INC) $^ -o $@

$(TARGET_CLIENT): $(SRC_CLIENT)
	$(CC) $(CFLAGS) $(INC) $^ -o $@

clean:
	rm -f $(TARGET_ENGINE) $(TARGET_CLIENT) *.o

.PHONY: all clean