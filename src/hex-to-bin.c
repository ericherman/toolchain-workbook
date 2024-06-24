// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Eric Herman <eric@freesa.org>

#include <stddef.h>
#include <stdint.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HEX_TO_BIN_VERSION "0.0.0"

#ifndef fdebugf
#define fdebugf(stream, format, ...) \
	do { \
		fprintf(stream, "%s %s() %d:", __FILE__, __func__, __LINE__); \
		fprintf(stream, format __VA_OPT__(,) __VA_ARGS__); \
		fprintf(stream, "\n"); \
	} while (0)
#endif

void print_help(FILE *out, const char *name)
{
	fprintf(out, "Usage: %s [OPTIONS]\n", name);
	fprintf(out, "Convert hex to binary.\n\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -i, --in FILE     Input file (default: stdin)\n");
	fprintf(out, "  -o, --out FILE    Output file (default: stdout)\n");
	fprintf(out, "  -V, --version     Show version (%s)\n",
		HEX_TO_BIN_VERSION);
	fprintf(out, "  -h, --help        Show this help message\n");
}

void print_version(FILE *out, const char *name)
{
	fprintf(out, "%s version %s\n", name, HEX_TO_BIN_VERSION);
}

uint8_t hex_char_to_nibble(char c)
{
	// range initializer are an extension supported by gcc & clang
	// we can unroll these if a compiler does not support this
	static const ssize_t hex_table[256] = {
		[0 ... ('0' - 1)] = -1,
		['0'] = 0,
		['1'] = 1,
		['2'] = 2,
		['3'] = 3,
		['4'] = 4,
		['5'] = 5,
		['6'] = 6,
		['7'] = 7,
		['8'] = 8,
		['9'] = 9,
		[('9' + 1) ... ('A' - 1)] = -1,
		['A'] = 10,
		['B'] = 11,
		['C'] = 12,
		['D'] = 13,
		['E'] = 14,
		['F'] = 15,
		[('F' + 1) ... ('a' - 1)] = -1,
		['a'] = 10,
		['b'] = 11,
		['c'] = 12,
		['d'] = 13,
		['e'] = 14,
		['f'] = 15,
		[('f' + 1) ... 255] = -1
	};

	uint8_t nibble = (uint8_t)hex_table[(size_t)c];

	return nibble;
}

uint8_t hex_to_byte(char hi, char lo)
{
	assert(isxdigit(hi));
	assert(isxdigit(lo));
	uint8_t byte = (hex_char_to_nibble(hi) << 4) | hex_char_to_nibble(lo);
	return byte;
}

int hex_to_bin(FILE *in, FILE *out, FILE *err)
{
	int error = 0;
	unsigned int byte = 0;
	size_t pos = 0;
	int ch = 0;
	char hex[3] = { '\0', '\0', '\0' };

	pos = 0;
	while ((ch = fgetc(in)) != EOF) {
		if (isspace(ch)) {
			continue;
		}

		if (ch == '#') {
			while ((ch = fgetc(in)) != EOF && ch != '\n') {
				// Skip the rest of the comment line.
			}
			continue;
		}

		if (!isxdigit(ch)) {
			fprintf(err, "Error: Invalid hex '%c'.\n", ch);
			error = 1;
			goto hex_to_bin_end;
		}

		hex[pos++] = ch;

		if (pos > 1) {
			byte = hex_to_byte(hex[0], hex[1]);
			fputc(byte, out);

			hex[0] = '\0';
			hex[1] = '\0';
			pos = 0;
		}
	}
	if (pos) {
		fprintf(err, "Error: leftover nibble? '%s'.\n", hex);
		error = 1;
	}

hex_to_bin_end:
	fflush(out);
	return error;
}

int hex_to_bin_opts(FILE *in, FILE *out, FILE *err, int argc, char *argv[])
{
	int error = 0;
	FILE *infile = in;
	FILE *outfile = out;
	int opt = 0;

	const char *opts = "i:o:Vh";
	static struct option longopts[] = {
		{ "in", required_argument, 0, 'i' },
		{ "out", required_argument, 0, 'o' },
		{ "version", no_argument, 0, 'V' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, opts, longopts, NULL)) != -1) {
		switch (opt) {
		case 'i':
			if (strcmp(optarg, "-") != 0) {
				infile = fopen(optarg, "r");
				if (!infile) {
					fprintf(err, "fopen %s\n",
						strerror(errno));
					error = 1;
					goto hex_to_bin_opts_end;
				}
			}
			break;
		case 'o':
			if (strcmp(optarg, "-") != 0) {
				outfile = fopen(optarg, "wb");
				if (!outfile) {
					fprintf(err, "fopen(%s) %s\n", optarg,
						strerror(errno));
					error = 1;
					goto hex_to_bin_opts_end;
				}
			}
			break;
		case 'V':
			print_version(out, argv[0]);
			goto hex_to_bin_opts_end;
		case 'h':
			print_help(out, argv[0]);
			goto hex_to_bin_opts_end;
		default:
			print_help(err, argv[0]);
			error = 1;
			goto hex_to_bin_opts_end;
		}
	}

	error = hex_to_bin(infile, outfile, err);

hex_to_bin_opts_end:
	if (infile && infile != in) {
		fclose(infile);
	}
	if (outfile && outfile != out) {
		fclose(outfile);
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
