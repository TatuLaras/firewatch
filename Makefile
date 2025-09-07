#
# Build firewatch reload utility 'fr'
#
NAME = fr
CC = gcc
INCLUDE = -I. -Iinclude
ADDITIONAL_CFLAGS =
CFLAGS = $(PACKAGES) $(INCLUDE) -Wall -Wextra -Wshadow -Wstrict-prototypes -march=native $(ADDITIONAL_CFLAGS)
CFLAGS_SHARED = $(CFLAGS) -fpic

SRC = $(wildcard src/*.c)
BUILD_DIR = build
SRC_DIR = src


$(BUILD_DIR)/fr: $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/fr_asan: $(SRC)
	$(CC) $(CFLAGS) -ggdb -fsanitize=address $^ -o $@

ARGS =
run: $(BUILD_DIR)/fr
	@echo -e "\n\n--------------------"
	@$(BUILD_DIR)/fr $(ARGS)

run_asan: $(BUILD_DIR)/fr_asan
	@echo -e "\n\n--------------------"
	@$(BUILD_DIR)/fr_asan $(ARGS)


install: $(BUILD_DIR)/fr
	sudo cp $(BUILD_DIR)/fr /usr/bin
