#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "../include/sender.h"
#include "../include/receiver.h"

#define BAUDRATE 9600
#define N_TRIES 3
#define TIMEOUT 3
typedef enum {
    SENDER,
    RECEIVER
} RunMode;

int main(int argc, char *argv[]) {

    if (argc < 3)
    {
        printf("[ERROR] Incorrect program usage.\n"
               "Usage: %s <SerialPort> <mode> <filename>\n"
               "Example: %s /dev/ttyS1 sender penguin.gif\n"
               "Available modes:\n"
               "\tsender\n"
               "\treceiver\n",
               argv[0],
               argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serialPort = argv[1];
    const char *mode_cmp = argv[2];

    const char *receiver_mode = "receiver";
    const char *sender_mode = "sender";

    RunMode mode;

    if (!strcmp(mode_cmp, receiver_mode)) mode = RECEIVER;
    else if (!strcmp(mode_cmp, sender_mode)) mode = SENDER;
    else {
        printf("[ERROR] Invalid mode.\n");
        exit(EXIT_FAILURE);
    }
    
    if (argc < 4 && mode == SENDER) {
        printf("[ERROR] Must specify file as sender.\n");
        exit(EXIT_FAILURE);
    }

    char *filename = NULL;

    if (argc == 4 || mode == SENDER) filename = argv[3];

    printf("[INFO] Starting link-layer protocol application\n"
           "  - Serial port: %s\n"
           "  - Role: %s\n"
           "  - Baudrate: %d\n"
           "  - Number of tries: %d\n"
           "  - Timeout: %d\n"
           "  - Filename: %s\n",
           serialPort,
           mode_cmp,
           BAUDRATE,
           N_TRIES,
           TIMEOUT,
           filename);

    int err;

    switch (mode) {
        case SENDER:
            err = handle_sender(serialPort, (int)mode, BAUDRATE, N_TRIES, TIMEOUT, filename);
            break;
        case RECEIVER:
            err = handle_receiver(serialPort, (int)mode, BAUDRATE, N_TRIES, TIMEOUT, filename);
            break;
    }

    return err;
}