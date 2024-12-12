#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>

#include "url_params.h"

#define USERPASS_FIELD 1
#define USERPASS_FIELD2 2
#define USERPASS_USER 3
#define USERPASS_PASS 4
#define USERONLY_USER 5
#define HOSTNAME 6
#define PATH 7

#define CHECK_MATCH(match) (match.rm_so != -1 && match.rm_eo != -1)

void fill_field(char *str, regmatch_t *match, char **field);

/* Parses the URL passed in by filling in the structure provided in `match`
   alongside the filename in its tail and returns 0 on success, -1 otherwise.
   Memory is only allocated on success, not needing any freeing in failure. */
int parse_url(char *url, url_params *params, char **filename);
long parse_port(char *buffer);

#endif