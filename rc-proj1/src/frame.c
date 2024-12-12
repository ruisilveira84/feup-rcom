#include <math.h>
#include <string.h>

#include "../include/frame.h"

#define DATA_CONTROL 1
#define CONTROL_FILESIZE 0
#define CONTROL_FILENAME 1

int info_frame_counter = 0;

int build_info_frame(unsigned char *buffer, int bufSize);

int build_frame(FrameType type, unsigned char *buffer, int bufSize)
{
    buffer[0] = SF_FLAG;
    buffer[1] = SENDER_ADDRESS;
    buffer[bufSize - 1] = SF_FLAG;

    switch (type) {
        case SET:
        {
            buffer[2] = SET_CONTROL;
            break;
        }
        case UA:
        {
            buffer[2] = UA_CONTROL;
            break;
        }
        case RR0:
        {
            buffer[2] = RR0_CONTROL;
            break;
        }
        case RR1:
        {
            buffer[2] = RR1_CONTROL;
            break;
        }
        case REJ0:
        {
            buffer[2] = REJ0_CONTROL;
            break;
        }
        case REJ1:
        {
            buffer[2] = REJ1_CONTROL;
            break;
        }
        case DISC:
        {
            buffer[2] = DISC_CONTROL;
            break;
        }
        case INFO:
        {
            buffer[2] = (info_frame_counter++ % 2) << 6;
            break;
        }
        default:
            return -1;
    }

    buffer[3] = BLOCK_CHECK(buffer[2]);
    
    return 0; 
}

void _sm_start(const unsigned char data)
{
    if (data == SF_FLAG) sm.state = FLAG_RCV;
}

void _sm_flag_rcv(const unsigned char data)
{
    switch (data)
    {
        case SENDER_ADDRESS:
        {
            sm.state = A_RCV;
            return;
        }
        case SF_FLAG:
        {
            return;
        }
        default:
        {
            sm.state = START;
            return;
        }
    }
}

void _sm_a_rcv(const unsigned char data)
{
    switch (data)
    {
        case SF_FLAG:
        {
            sm.state = FLAG_RCV;
            return;
        }
        case SET_CONTROL:
        case UA_CONTROL:
        case RR0_CONTROL:
        case RR1_CONTROL:
        case REJ0_CONTROL:
        case REJ1_CONTROL:
        case INFO0_CONTROL:
        case INFO1_CONTROL:
        case DISC_CONTROL:
        {
            sm.state = C_RCV;
            rcv_control = data;
            return;
        }
        default:
        {
            sm.state = START;
            return;
        }
    }
}

void _sm_c_rcv(const unsigned char data)
{
    switch (data)
    {
        case SF_FLAG:
        {
            sm.state = FLAG_RCV;
            return;
        }
        case BLOCK_CHECK(SET_CONTROL):
        case BLOCK_CHECK(UA_CONTROL):
        case BLOCK_CHECK(RR0_CONTROL):
        case BLOCK_CHECK(RR1_CONTROL):
        case BLOCK_CHECK(REJ0_CONTROL):
        case BLOCK_CHECK(REJ1_CONTROL):
        case BLOCK_CHECK(DISC_CONTROL):
        {
            sm.state = BCC_OK;
            return;
        }
        case BLOCK_CHECK(INFO0_CONTROL):
        case BLOCK_CHECK(INFO1_CONTROL):
        {
            if (BLOCK_CHECK(rcv_control) == data) {
                sm.state = BCC_OK;
                return;
            }
        }
        default:
            sm.state = START;
            return;
    }
}

void _sm_bcc_ok(const unsigned char data)
{
    switch (data)
    {
        case SF_FLAG:
        {
            sm.state = STOP;
            return;
        }
        default:
            if (rcv_control == INFO0_CONTROL ||
                rcv_control == INFO1_CONTROL)
                return;

            sm.state = START;
            return;
    }
}
