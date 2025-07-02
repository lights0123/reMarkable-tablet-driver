#include "argument_parser.h"

const char *argp_program_bug_address =
    "https://github.com/FreeCap23/reMarkable-tablet-driver/issues";

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key) {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'a':
      /* 192.168.xxx.xxx = 15 characters */
      if (strlen(arg) > 15) {
        fprintf(
            stderr,
            "Address %s is too long or doesn't match the format required.\n",
            arg);
        return ARGP_ERR_UNKNOWN;
      }
      arguments->address = arg;
      break;
    case 'k':
      arguments->private_key_file = arg;
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

void print_arguments(struct arguments *args) {
  printf(
      "verbose = %s\n"
      "private key file = %s\n"
      "address = %s\n",
      args->verbose ? "yes" : "no", args->private_key_file, args->address);
}