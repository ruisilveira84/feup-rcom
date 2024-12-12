#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "format.h"
#include "utils.h"
#include "ftp.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("[ERROR] Please provide URL.\n");
        printf("Usage: %s ftp://[user:pass@]host/path\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    url_params params;
    memset(&params, 0, sizeof(url_params));

    char *filename = NULL;

    // No memory allocated on failure
    if (parse_url(argv[1], &params, &filename) < 0) exit(EXIT_FAILURE);

    if (params.username != NULL && params.password == NULL) params.password = get_password();

    format_url(&params);

    int return_code = ftp(&params, filename);

    free_params(&params);
    free(filename);
    
    return return_code == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
