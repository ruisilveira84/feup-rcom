#include "url_params.h"

#include <stdlib.h>

void free_params(url_params *params) {
    free(params->username);
    free(params->password);
    free(params->hostname);
    free(params->path);
}