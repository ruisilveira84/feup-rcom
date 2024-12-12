#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/sender.h"
#include "../include/serialio.h"
#include "../include/alarm.h"
#include "../include/packet.h"
#include "../include/error.h"

#define ALARM_TIMER 3
#define MAX_RETRIES 3

#define EXIT_FAILURE  1
#define EXIT_SUCCESS  0

int packet_data_size;
unsigned char *file_content_buffer;

int handle_sender(const char *serialPort, int role, int baudRate,
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

    FILE *source = fopen(filename, "r");
    if (handle_fopen(source)) return -1;

    (void)fseek(source, 0L, SEEK_END);
    long size = ftell(source);
    (void)fseek(source, 0L, SEEK_SET);

    unsigned char buffer[MAX_PAYLOAD_SIZE];
    size_t bytes = build_packet(CONTROL, buffer, filename, size);
    if (handle_llwrite(llwrite(buffer, bytes))) return -1;

    packet_data_size = 512;
    file_content_buffer = buffer + DATA_OFFSET;

    while (!feof(source))
    {
        int read_bytes = fread(file_content_buffer, 1, packet_data_size, source);
        if (read_bytes < packet_data_size && ferror(source)) {
            printf("[ERROR] Failed reading file.\n");
            return -1;
        }

        packet_data_size = read_bytes;

        bytes = build_packet(DATA, buffer, NULL, 0);
        if (handle_llwrite(llwrite(buffer, bytes))) return -1;
    }

    bytes = build_packet(CONTROL, buffer, filename, size);
    if (handle_llwrite(llwrite(buffer, bytes))) return -1;

    fclose(source);

    if (handle_llclose(llclose())) return -1;
    printf("[INFO] Successfully disconnected.\n");
    return 0;
}

