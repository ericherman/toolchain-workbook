# SPDX-License-Identifier: LGPL-2.1-or-later
# Copyright (C) 2024 Eric Herman <eric@freesa.org>

CC = cc
AS = as
LD = ld

# CFLAGS = -g -O2 -DNDEBUG -Wall -Wextra -Wcast-qual -Wc++-compat
CFLAGS = -O2 -DNDEBUG -Wall -Wextra -Wcast-qual -Wc++-compat

DYNAMIC_LINKER ?= /usr/lib64/ld-linux-x86-64.so.2
LDFLAGS = -dynamic-linker $(DYNAMIC_LINKER) -lc -z noexecstack

SHELL=/bin/bash
START_S ?= src/start.$(shell echo $$(arch)-$$(uname|tr 'A-Z' a-z)).s

BUILD_DIR ?= build


.PHONY: all
all: $(BUILD_DIR)/hex-to-bin

$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)

$(BUILD_DIR)/start.o: $(START_S) | $(BUILD_DIR)
	$(AS) -o $@ $<

$(BUILD_DIR)/hex-to-bin.s: src/hex-to-bin.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILD_DIR)/hex-to-bin.o: $(BUILD_DIR)/hex-to-bin.s
	$(AS) -o $@ $<

$(BUILD_DIR)/hex-to-bin-opts.s: src/hex-to-bin-opts.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILD_DIR)/hex-to-bin-opts.o: $(BUILD_DIR)/hex-to-bin-opts.s
	$(AS) -o $@ $<

$(BUILD_DIR)/hex-to-bin-main.s: src/hex-to-bin-main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILD_DIR)/hex-to-bin-main.o: $(BUILD_DIR)/hex-to-bin-main.s
	$(AS) -o $@ $<

$(BUILD_DIR)/hex-to-bin: \
		$(BUILD_DIR)/hex-to-bin.o \
		$(BUILD_DIR)/hex-to-bin-opts.o \
		$(BUILD_DIR)/hex-to-bin-main.o \
		$(BUILD_DIR)/start.o
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test-nibbles.s: tests/test-nibbles.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -S $< -o $@

$(BUILD_DIR)/test-nibbles.o: $(BUILD_DIR)/test-nibbles.s
	$(AS) -o $@ $<

$(BUILD_DIR)/test-nibbles: \
		$(BUILD_DIR)/test-nibbles.o \
		$(BUILD_DIR)/hex-to-bin.o \
		$(BUILD_DIR)/start.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: check-nibbles
check-nibbles: $(BUILD_DIR)/test-nibbles
	$(BUILD_DIR)/test-nibbles
	@echo "SUCCESS $@"

.PHONY: check-round-trip
check-round-trip: build/hex-to-bin tests/test0.hex
	$< -i tests/test0.hex > $(BUILD_DIR)/test0.bin
	grep '^#' tests/test0.hex > $(BUILD_DIR)/test0.xxd.hex
	xxd -p $(BUILD_DIR)/test0.bin >> $(BUILD_DIR)/test0.xxd.hex
	diff -u tests/test0.hex $(BUILD_DIR)/test0.xxd.hex
	@echo "SUCCESS $@"

check-round-trip-comments: $(BUILD_DIR)/hex-to-bin \
		tests/all.hex tests/all.xxd.expect.hex
	$(BUILD_DIR)/hex-to-bin \
		--in=tests/all.hex \
		--out=$(BUILD_DIR)/all.hex.bin
	grep '^#' tests/all.hex > $(BUILD_DIR)/all.xxd.hex
	xxd -p $(BUILD_DIR)/all.hex.bin >> $(BUILD_DIR)/all.xxd.hex
	diff -u tests/all.xxd.expect.hex $(BUILD_DIR)/all.xxd.hex
	@echo "SUCCESS $@"

.PHONY: check
check: check-nibbles check-round-trip check-round-trip-comments
	@echo "SUCCESS $@"

.PHONY: tidy
tidy: bin/ctidy
	bin/ctidy $$(find src/ tests/ -name *.c -o -name *.h)

.PHONY: clean
clean:
	rm -rfv build
