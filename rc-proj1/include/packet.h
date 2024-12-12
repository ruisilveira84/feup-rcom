#ifndef _PACKET_H_
#define _PACKET_H_

#define DATA_OFFSET 3
#define PACKET_DATA_CONTROL        1
#define PACKET_START_CONTROL    2
#define PACKET_END_CONTROL        3
typedef enum {
    CONTROL,
    DATA
} PacketType;

extern unsigned char *file_content_buffer;
extern int packet_data_size;

size_t build_packet(PacketType type, unsigned char *buffer, const char *filename, long filesize);

#endif