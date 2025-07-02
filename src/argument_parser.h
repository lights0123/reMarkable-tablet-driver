#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <argp.h>
#include <stdlib.h>
#include <string.h>

/* Parse a single option. */
error_t parse_opt(int key, char *arg, struct argp_state *state);

extern const char *argp_program_bug_address;

/* Program documentation. */
static char doc[] =
    "reMarkable Tablet Driver -- a driver for using your reMarkable tablet as "
    "pen input";

/* The options we understand. */
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output."},
    {"key", 'k', "FILE", 0, "Private key used for authentication."},
    {"address", 'a', "ADDRESS", 0,
     "Address of reMarkable tablet. Default is 10.11.99.1."},
    {0}};

/* Our argp parser. */
static struct argp argp = {options, parse_opt, 0, doc};

/* Used by main to communicate with parse_opt. */
struct arguments {
  int verbose;
  char *private_key_file, *address;
};

/* Print the argument list */
void print_arguments(struct arguments *args);

#endif