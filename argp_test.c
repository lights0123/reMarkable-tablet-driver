#include "argument_parser.h"

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
