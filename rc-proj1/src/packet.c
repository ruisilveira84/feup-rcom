#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "../include/packet.h"

#define CONTROL_FILESIZE 0
#define CONTROL_FILENAME 1

int control_packet_counter = 0;

size_t build_packet(PacketType type, unsigned char *buffer, const char *filename, long filesize)
{
    switch (type) {
        case CONTROL:
        {
            int filesize_bytes = (int)ceil(log2((double)(filesize + 1)) / 8);
            int filename_bytes = strlen(filename) + 1;
            size_t packet_size = filesize_bytes + filename_bytes + 5;

            buffer[0] = (control_packet_counter++ % 2) + 2;        // 2 - start; 3 - end
            buffer[1] = CONTROL_FILESIZE;
            buffer[2] = filesize_bytes;
            for (int i = filesize_bytes; i > 0; i--)
            {
                buffer[3 + filesize_bytes - i] = (filesize >> (8 * (i - 1))) & 0xFF;
            }
            unsigned char next = 3 + filesize_bytes;
            buffer[next] = CONTROL_FILENAME;
            buffer[next + 1] = filename_bytes;
            (void*)strcpy((char*)(buffer + next + 2), filename);

            return packet_size;
        }
        case DATA:
        {
            buffer[0] = PACKET_DATA_CONTROL;
            buffer[1] = (packet_data_size >> 8) & 0xFF;
            buffer[2] = packet_data_size & 0xFF;
            (void*)memcpy(buffer + 3, file_content_buffer, packet_data_size);
            return packet_data_size + 3;
        }
        default:
            return -1;
    }

    return 0;
}