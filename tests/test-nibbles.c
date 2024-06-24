// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Eric Herman <eric@freesa.org>

#include <stdint.h>

#include <assert.h>
#include <stdio.h>

uint8_t hex_char_to_nibble(char c);
uint8_t hex_to_byte(char hi, char lo);

static char nibble_to_hex_char(uint8_t nibble)
{
	const char *hexchars = "0123456789abcdef";
	assert(nibble < 16);
	return hexchars[nibble];
}

static char nibble_to_hex_char_upper(uint8_t nibble)
{
	const char *hexchars_upper = "0123456789ABCDEF";
	assert(nibble < 16);
	return hexchars_upper[nibble];
}

unsigned test_hex_to_byte(void)
{
	unsigned err = 0;

	size_t i = 0;
	uint8_t byte = 0;

	for (i = 0; i <= 15; ++i) {
		char c = nibble_to_hex_char(i);
		byte = hex_char_to_nibble(c);
		if (byte != i) {
			fprintf(stderr, "expected %c == %02zx but was %02x\n",
				c, i, byte);
			++err;
		}
	}
	for (i = 10; i <= 15; ++i) {
		char c = nibble_to_hex_char_upper(i);
		byte = hex_char_to_nibble(c);
		if (byte != i) {
			fprintf(stderr, "expected %c == %02zx but was %02x\n",
				c, i, byte);
			++err;
		}
	}

	byte = hex_to_byte('7', 'f');
	if (byte != 0x7f) {
		fprintf(stderr, "expected 0x7f but was 0x%02x\n", byte);
		++err;
	}

	return err;
}

int main(void)
{
	unsigned errors = 0;

	errors += test_hex_to_byte();

	return errors ? 1 : 0;
}
