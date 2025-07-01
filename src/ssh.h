#ifndef SSH_H
#define SSH_H

#include <errno.h>
#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argument_parser.h"

/*
 * https://api.libssh.org/stable/libssh_tutor_guided_tour.html
 */
int verify_knownhost(ssh_session session);

/*
 * Authenticate using the pubkey from the SSH agent
 * https://api.libssh.org/stable/libssh_tutor_authentication.html#authentication_details
 */
int authenticate_pubkey(ssh_session session);

/*
 * Authenticate using a given private key
 */
int authenticate_privkey(ssh_session session, ssh_key privkey);

/*
 * Authenticate using the password.
 */
int authenticate_with_password(ssh_session session);

int print_command_output(ssh_session session, const char *cmd);

int create_ssh_session(ssh_session *session, const char *address,
                       const int *port);

#endif
