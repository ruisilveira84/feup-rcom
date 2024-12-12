#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/receiver.h"
#include "../include/serialio.h"
#include "../include/error.h"
#include "../include/packet.h"

#define EXIT_FAILURE  1
#define EXIT_SUCCESS  0

#define TRUE 1
#define FALSE 0

int handle_receiver(const char *serialPort, int role, int baudRate,
                    int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    memset(&ll, 0, sizeof(ll));

    (void*)strncpy(ll.serialPort, serialPort, 50);
    ll.role = (LinkLayerRole)role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;
 
    if (handle_llopen(llopen(&ll))) return -1;

    FILE *dest = NULL;

    if (filename != NULL) {
        dest = fopen(filename, "w");
        if (handle_fopen(dest)) return -1;
    }

    unsigned char buffer[MAX_PAYLOAD_SIZE];

    int bytes;
    do {
        bytes = llread(buffer);
        if (handle_llread(bytes)) return -1;
    } while (bytes == 0);

    if (buffer[0] != PACKET_START_CONTROL) return -1;
    
    unsigned char filesize_size = buffer[2];
    long filesize = 0;

    for (int i = 0; i < filesize_size; i ++) {
        filesize += (buffer[3 + i] << (8 * (filesize_size - i - 1)));
    }

    unsigned char filename_size = buffer[4 + filesize_size];
    char filename_cmp[50] = {'\0'};
    (void*)strncpy(filename_cmp, (char*)(buffer + filesize_size + 5), filename_size);

    if (dest == NULL) {
        dest = fopen(filename_cmp, "w");
        if (handle_fopen(dest)) return -1;
    }


    do {
        bytes = llread(buffer);
        if (handle_llread(bytes)) return -1;
        if (bytes == 0) continue;

        switch (buffer[0]) {
            case PACKET_DATA_CONTROL:
            {
                int data_size = 256 * buffer[1] + buffer[2];
                (void)fwrite(buffer + 3, 1, data_size, dest);
                break;
            }
            case PACKET_END_CONTROL:
            {
                (void)fclose(dest);
                goto finish;
            }
        }
    } while (1);

finish:

    if (handle_llclose(llclose())) return -1;
    printf("[INFO] Successfully disconnected.\n");
    return 0;
}
