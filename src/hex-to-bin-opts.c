// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Eric Herman <eric@freesa.org>

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEX_TO_BIN_VERSION "0.0.0"

int hex_to_bin(FILE *in, FILE *out, FILE *err);

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
