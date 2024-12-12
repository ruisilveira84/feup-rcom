#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "format.h"

#define ADDED_CHARACTERS 7

char *format(char *field) {
    const char *reserved = "!#$&'()*+,/:;=?@[]";

    size_t length = strlen(field);
    size_t reserved_length = strlen(reserved);

    int counter = 0;

    for (size_t i = 0; i < length; i++) {
        for (size_t j = 0; j < reserved_length; j++) {
            if (field[i] != reserved[j]) continue;
            counter++;
            break;
        }
    }
    
    size_t new_length = length + 2 * counter + 1;
    char *ret = calloc(new_length, sizeof(char));

    counter = 0;

    for (size_t i = 0; i < length; i++) {
        bool is_reserved = false;
        for (size_t j = 0; j < reserved_length; j++) {
            if (field[i] != reserved[j]) continue;
            ret[i + 2 * counter] = '%';
            sprintf(&ret[i + 2 * counter + 1], "%X", reserved[j]);
            counter++;
            is_reserved = true;
            break;
        }
        if (!is_reserved) ret[i + 2 * counter] = field[i];
    }

    free(field);
    return ret;
}

void format_url(url_params *params) {
    params->hostname = format(params->hostname);
    params->path = format(params->path);
    params->username = format(params->username);
    params->password = format(params->password);
}
