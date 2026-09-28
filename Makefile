CC       := gcc
SRC      := main.c utils.c
BUILD_DIR := build

PREFIX   := /usr/local
DESTDIR  :=

# Suckless-style config: config.h overrides config.def.h
# If config.h doesn't exist, copy config.def.h to config.h
CONFIG_H := config.h
CONFIG_DEF_H := config.def.h

# Ensure config.h exists (copy from config.def.h if needed)
$(CONFIG_H): $(CONFIG_DEF_H)
	@if [ ! -f $(CONFIG_H) ]; then cp $(CONFIG_DEF_H) $(CONFIG_H); fi

COMMON_CFLAGS := -no-pie -I./ \
                 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wnull-dereference \
                 -fstack-protector-strong

PROD_CFLAGS   := $(COMMON_CFLAGS) -O3 -march=native -DNDEBUG
PROD_LDFLAGS  := -s
PROD_TARGET   := $(BUILD_DIR)/nw-status

RELEASE_CFLAGS := $(COMMON_CFLAGS) -O2 -DNDEBUG
RELEASE_LDFLAGS := -s
RELEASE_TARGET := $(BUILD_DIR)/nw-status-release

DEBUG_CFLAGS  := $(COMMON_CFLAGS) -O0 -g3 \
                -fsanitize=address,undefined
DEBUG_LDFLAGS := -fsanitize=address,undefined
DEBUG_TARGET  := $(BUILD_DIR)/nw-status-debug

.PHONY: all release debug install install-release uninstall clean

all: $(PROD_TARGET)

release: $(RELEASE_TARGET)

debug: $(DEBUG_TARGET)

$(PROD_TARGET): $(SRC) $(CONFIG_H) | $(BUILD_DIR)
	$(CC) $(PROD_CFLAGS) -o $@ $(SRC) $(PROD_LDFLAGS)

$(RELEASE_TARGET): $(SRC) $(CONFIG_H) | $(BUILD_DIR)
	$(CC) $(RELEASE_CFLAGS) -o $@ $(SRC) $(RELEASE_LDFLAGS)

$(DEBUG_TARGET): $(SRC) $(CONFIG_H) | $(BUILD_DIR)
	$(CC) $(DEBUG_CFLAGS) -o $@ $(SRC) $(DEBUG_LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PROD_TARGET) $(DESTDIR)$(PREFIX)/bin/nw-status

install-release: release
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(RELEASE_TARGET) $(DESTDIR)$(PREFIX)/bin/nw-status

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/nw-status

clean:
	rm -rf $(BUILD_DIR)