CC = gcc
SANITIZE = -fsanitize=address
CFLAGS = -Wall -Wextra -Wshadow -pedantic -Wstrict-prototypes -march=native -pthread -lm
CFLAGS_DEBUG = $(CFLAGS) -DDEBUG -ggdb
CFLAGS_ASAN = $(CFLAGS) -DDEBUG $(SANITIZE)
BUILD_DIR = build

ARGS =

run: $(BUILD_DIR) $(BUILD_DIR)/debug
	@echo -e "Warning: no address sanitation enabled, consider running with 'make run_asan' when developing.\n\n\n"
	$(BUILD_DIR)/debug $(ARGS)

run_asan: $(BUILD_DIR) $(BUILD_DIR)/asan
	@echo -e "\n\n\n"
	$(BUILD_DIR)/asan $(ARGS)


SRC = $(wildcard *.c)

$(BUILD_DIR)/debug: $(SRC)
	@echo "Building debug build"
	$(CC) -o $@ $^ $(CFLAGS_DEBUG)

$(BUILD_DIR)/asan: $(SRC)
	@echo "Building address sanitation build"
	$(CC) -o $@ $^ $(CFLAGS_ASAN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
