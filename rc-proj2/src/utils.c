#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MAX_PASSWORD_SIZE 128

char *get_password(void) {
    struct termios old, new;

    tcgetattr(STDIN_FILENO, &old);
    new = old;

    new.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    printf("Password: ");
    fflush(stdout);

    char *password = calloc(MAX_PASSWORD_SIZE + 1, sizeof(char));
    if (fgets(password, MAX_PASSWORD_SIZE + 1, stdin) == NULL) password[0] = '\0';
    else password[strlen(password) - 1] = '\0';

    printf("\n");

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return password;
}