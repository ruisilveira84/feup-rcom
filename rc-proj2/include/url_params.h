#ifndef URL_PARAMS_H
#define URL_PARAMS_H

typedef struct {
    char *username;
    char *password;
    char *hostname;
    char *path;
} url_params;

void free_params(url_params *params);

#endif