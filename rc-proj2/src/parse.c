#include "parse.h"
#include "utils.h"

#include <string.h>

void fill_field(char *str, regmatch_t *match, char **field){
    int fieldLength = match->rm_eo - match->rm_so;
    // Null terminator
    *field = calloc(fieldLength + 1, sizeof(char));
    (void)strncpy(*field, &str[match->rm_so], fieldLength);
}

int parse_url(char *url, url_params *params, char** filename) {

    int return_code = 0;

    if (url == NULL) {
        printf("[ERROR] Please supply an FTP URL.\n");
        return_code = -1; goto url_exit;
    }

    regex_t result;
    memset(&result, 0, sizeof(regex_t));

    /* Pattern explanation (capture groups):
       If 1 and 2 are not empty, 3 and 4 are username and password, respectively
       If 1 or 2 are empty, 5 is username
       6 is host
       7 is path
       Allowed: user:pass@; user:@; user@; :@; 
       Otherwise, whichever characters are counted as part of hostname */
    const char *pattern = "ftp://(((.*):(.*)@)|(.*)@)?([^/]*)/(.*)";

    if (regcomp(&result, pattern, REG_EXTENDED)) {
        printf("[ERROR] Failed to compile regular expression.\n");
        return_code = -1; goto url_free_preg;
    }

    regmatch_t *matches = malloc((result.re_nsub + 1) * sizeof(regmatch_t));

    // Added one since it automatically matches the whole string at index zero
    // re_nsub has subexpressions; matches return whole expression + subexpressions
    if (regexec(&result, url, result.re_nsub + 1, matches, 0)) {
        printf("[ERROR] Invalid URL. Please recheck and try again.\n");
        return_code = -1; goto url_free_matches;
    }

    // Username and password provided
    if (CHECK_MATCH(matches[USERPASS_FIELD]) &&
        CHECK_MATCH(matches[USERPASS_FIELD2])) {
        fill_field(url, &matches[USERPASS_USER], &params->username);

        if (strlen(params->username) == 0) {
            printf("[ERROR] Username field is empty.\n");
            free(params->username);
            return_code = -1; goto url_free_matches;
        }
        
        fill_field(url, &matches[USERPASS_PASS], &params->password);

        if (strlen(params->password) == 0) {
            printf("[ERROR] Password field is empty.\n");
            free(params->password);
            return_code = -1; goto url_free_matches;
        }

    // Only username provided
    } else if (CHECK_MATCH(matches[USERONLY_USER])) {
        fill_field(url, &matches[USERONLY_USER], &params->username);
    } else {
        params->username = calloc(10, sizeof(char));
        (void)sprintf(params->username, "%s", "anonymous");
        params->password = calloc(10, sizeof(char));
        (void)sprintf(params->password, "%s", "anonymous");
    }

    fill_field(url, &matches[HOSTNAME], &params->hostname);
    fill_field(url, &matches[PATH], &params->path);

    size_t pos = 0;
    for (size_t i = 0; i < strlen(params->path); i++) if (params->path[i] == '/') pos = i;

    *filename = calloc(strlen(&params->path[pos]) + 1, sizeof(char));
    (void)strcpy(*filename, &params->path[pos]);

url_free_matches:
    free(matches);
url_free_preg:
    regfree(&result);
url_exit:
    return return_code;
}

long parse_port(char *buffer) {

    long ret;

    regex_t result;
    memset(&result, 0, sizeof(regex_t));

    const char pattern[] = ".*\\(([0-9]+,){4}(([0-9]+),([0-9]+))\\).";

    if (regcomp(&result, pattern, REG_EXTENDED)) {
        printf("[ERROR] Failed to compile regular expression.\n");
        ret = -1; goto port_free_preg;
    }

    regmatch_t *matches = malloc((result.re_nsub + 1) * sizeof(regmatch_t));

    if (regexec(&result, buffer, result.re_nsub + 1, matches, 0)) {
        printf("[ERROR] Could not retrieve valid port from server.\n");
        ret = -1; goto port_free_matches;
    }

    ret = (strtol(&buffer[matches[3].rm_so], NULL, 10) << 8) +
           strtol(&buffer[matches[4].rm_so], NULL, 10);

port_free_matches:
    free(matches);
port_free_preg:
    regfree(&result);

    return ret;
}