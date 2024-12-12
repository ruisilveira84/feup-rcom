#include "../include/error.h"

int handle_llopen(int err)
{
    if (err != -1) return 0;
    printf("[ERROR] Failed to establish connection.\n");
    return -1;
}

int handle_llwrite(int err)
{
    if (err != -1) return 0;
    printf("[ERROR] Failed to send frame.\n");
    return -1;
}

int handle_llread(int err)
{
    if (err != -1) return 0;
    printf("[ERROR] Failed to receive frame.\n");
    return -1;
}

int handle_llclose(int err)
{
    if (err != -1) return 0;
    printf("[ERROR] Failed to disconnect.\n");
    return -1;
}

int handle_fopen(FILE *err)
{
    if (err != NULL) return 0;

    printf("[ERROR] Failed to open the file.\n");

    #ifdef DEBUG
    perror("[DEBUG] fopen");
    #endif

    return -1;
}

int handle_open(int err)
{
    if (err != -1) return 0;

    printf("[ERROR] Unable to open serial port device.\n");

    #ifdef DEBUG
    perror("[DEBUG] open");
    #endif

    return -1;
}

int handle_read(int err)
{
    if (err != -1) return 0;

    printf("[ERROR] Unable to read from file descriptor.\n");

    #ifdef DEBUG
    perror("[DEBUG] read");
    #endif

    return -1;
}

int handle_write(int err)
{
    if (err != -1) return 0;

    printf("[ERROR] Failed to write to file descriptor");

    #ifdef DEBUG
    perror("[DEBUG] write");
    #endif

    return -1;
}

int handle_tcgetattr(int err)
{
    if (err != -1) return 0;

    printf("[ERROR] Unable to save current port settings.");
    
    #ifdef DEBUG
    perror("[DEBUG] tcgetattr");
    #endif
    
    return -1;
}

int handle_tcsetattr(int err, int new)
{
    if (err != -1) return 0;
    if (new) printf("[ERROR] Failed to set new port settings.\n");
    else printf("[ERROR] Failed to restore the old port settings.\n");
    
    #ifdef DEBUG
    perror("[DEBUG] tcsetattr");
    #endif

    return -1;
}
