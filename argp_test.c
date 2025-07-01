#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include "argp_test.h"

const char *argp_program_bug_address =
  "https://github.com/FreeCap23/reMarkable-tablet-driver/issues";

/* Program documentation. */
static char doc[] =
  "reMarkable Tablet Driver -- a driver for using your reMarkable tablet as pen input";

/* The options we understand. */
static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"key", 'k', "FILE", 0, "Private key used for authentication"},
  {"address", 'a', "ADDRESS", 0, "Address of reMarkable tablet. Default is 10.11.99.1"},
  {"orientation", 'o', "ORIENTATION", 0, "Tablet orientation. Valid options are top, left, right, bottom, corresponding to the position of the tablet buttons. Case sensitive! Default is right."},
  {"threshold", 't', "THRESHOLD", 0, "Pen pressure threshold. Default is 600"},
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments {
  int verbose, threshold;
  char *private_key_file, *address, *orientation;
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key) {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'k':
      arguments->private_key_file = arg;
      break;
    case 'a':
      arguments->address = arg;
      break;
    case 'o':
      if (
        strcmp(arg, "top") == 0 ||
        strcmp(arg, "buttom") == 0 ||
        strcmp(arg, "right") == 0 ||
        strcmp(arg, "left") == 0) {
          arguments->orientation = arg;
      } else {
        fprintf(stderr, "Wrong argument key %s. Is it one of top, left, right, bottom?\n", arg);
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 't':
      arguments->threshold = atoi(arg);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Print the argument list */
void print_arguments(struct arguments *args) {
  printf ("verbose = %s\n"
          "private key file = %s\n"
          "address = %s\n"
          "orientation = %s\n"
          "threshold = %d\n",
          args->verbose ? "yes" : "no",
          args->private_key_file,
          args->address,
          args->orientation,
          args->threshold
        );

}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, 0, doc };

int main (int argc, char **argv) {
  struct arguments arguments;

  /* Default values. */
  arguments.verbose = 0;
  arguments.private_key_file = "";
  arguments.address = "10.11.99.1";
  arguments.orientation = "right";
  arguments.threshold = 600;

  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  if (arguments.verbose) {
    print_arguments(&arguments);
  }
  exit (0);
}
