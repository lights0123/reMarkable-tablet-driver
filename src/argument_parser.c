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
    case 'k':
      /* 192.168.xxx.xxx = 15 characters */
      if (strlen(arg) > 15) {
        fprintf(
            stderr,
            "Address %s is too long or doesn't match the format required.\n",
            arg);
        return ARGP_ERR_UNKNOWN;
      }
      arguments->private_key_file = arg;
      break;
    case 'a':
      arguments->address = arg;
      break;
    case 'p':
      arguments->port = atoi(arg);
      break;
    case 'o':
      if (strcmp(arg, "top") == 0 || strcmp(arg, "bottom") == 0 ||
          strcmp(arg, "right") == 0 || strcmp(arg, "left") == 0) {
        arguments->orientation = arg;
      } else {
        fprintf(stderr,
                "Wrong argument key %s. Is it one of top, left, right, "
                "bottom?\n",
                arg);
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

void print_arguments(struct arguments *args) {
  printf(
      "verbose = %s\n"
      "private key file = %s\n"
      "address = %s\n"
      "port = %d\n"
      "orientation = %s\n"
      "threshold = %d\n\n",
      args->verbose ? "yes" : "no", args->private_key_file, args->address,
      args->port, args->orientation, args->threshold);
}