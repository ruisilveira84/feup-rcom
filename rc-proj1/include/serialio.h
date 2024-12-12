#ifndef _SERIALIO_H_
#define _SERIALIO_H_

#include "frame.h"

#define MAX_PAYLOAD_SIZE 1000

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
} LinkLayer;

int llopen(LinkLayer *connParam);
int llwrite(const unsigned char *buffer, int bufSize);
int llread(unsigned char *packet);
int llclose(void);

#endif
