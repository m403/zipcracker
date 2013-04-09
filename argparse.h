#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <getopt.h>
#include <stdio.h>

int optparse(int, char **, char **, char **);
void print_usage(FILE *, int);

#endif
