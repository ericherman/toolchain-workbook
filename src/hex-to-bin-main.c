// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Eric Herman <eric@freesa.org>

#include <stdio.h>

int hex_to_bin_opts(FILE *in, FILE *out, FILE *err, int argc, char **argv);

int main(int argc, char **argv)
{
	return hex_to_bin_opts(stdin, stdout, stderr, argc, argv);
}
