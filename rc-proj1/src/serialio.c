#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../include/serialio.h"
#include "../include/alarm.h"
#include "../include/error.h"

#define ESCAPE 0x7D
#define STUFF(n) (n ^ 0x20)
#define DESTUFF(n) STUFF(n)

StateMachine sm;
int fd;
unsigned char rcv_control;
LinkLayer ll;
struct termios oldtio;

int llopen(LinkLayer *connParam)
{
    ll = *connParam;

    // Set port config
    fd = open(connParam->serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    
    if (handle_open(fd)) exit(EXIT_FAILURE);

    struct termios newtio;

    // Save current port settings
    if (handle_tcgetattr(tcgetattr(fd, &oldtio))) exit(EXIT_FAILURE);

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connParam->baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode
    newtio.c_lflag = 0;         // Non-canonical
    newtio.c_cc[VTIME] = 0;     // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;      // Non-blocking read

    // Flush data received but not read
    tcflush(fd, TCIOFLUSH);
    
    // Set new port settings
    if (handle_tcsetattr(tcsetattr(fd, TCSANOW, &newtio), TRUE)) exit(EXIT_FAILURE);

    printf("[INFO] New termios structure set.\n");
    
    // Set up state machine
    sm.transition = calloc(5, sizeof(_trans_func));
    
    sm.transition[START] = _sm_start;
    sm.transition[FLAG_RCV] = _sm_flag_rcv;
    sm.transition[A_RCV] = _sm_a_rcv;
    sm.transition[C_RCV] = _sm_c_rcv;
    sm.transition[BCC_OK] = _sm_bcc_ok;
    
    sm.state = START;
    
    printf("[INFO] State machine set up.\n");

    (void)signal(SIGALRM, alarmHandler);

    switch (connParam->role)
    {
        case LlTx:
        {
            unsigned char frame[SU_FRAME_SIZE];
            build_frame(SET, frame, SU_FRAME_SIZE);
            if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;
            while (alarmCount < connParam->nRetransmissions)
            {
                unsigned char packet;
                if (handle_read(read(fd, &packet, 1))) return -1;
                sm.transition[sm.state](packet);
                if (sm.state == STOP && rcv_control != UA_CONTROL) sm.state = START;
                else if (sm.state == STOP) break;
                if (alarmEnabled) continue;
                if (alarmCount > 0) {
                        printf("[INFO] Resending SET frame to attempt connection... (Tries left: %d)\n",
                                connParam->nRetransmissions - alarmCount + 1);
                        if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;
                }
                alarm(connParam->timeout);
                alarmEnabled = TRUE;
            }
            alarm(0);
            if (rcv_control != UA_CONTROL) return -1;
            if (alarmCount < connParam->nRetransmissions) return 0;
            printf("[INFO] Max retries reached. Timed out.\n");
            return -1;
        }
        case LlRx:
        {
			// Receiver is supposed to wait
            // alarm(ll.timeout);
            // alarmEnabled = TRUE;

            while (sm.state != STOP)
            {
                // if (alarmEnabled == FALSE) return -1;

                unsigned char packet;
                int bytes = read(fd, &packet, 1);
                if (handle_read(bytes)) return -1;
                
                if (bytes == 0) continue;
                sm.transition[sm.state](packet);
            }

            // alarm(0);
            // alarmCount = 0;
            // alarmEnabled = FALSE;

            unsigned char frame[SU_FRAME_SIZE];
            build_frame(UA, frame, SU_FRAME_SIZE);
            if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;

            return 0;
        }
    }
    return -1;
}

int llwrite(const unsigned char *buffer, int bufSize)
{
    sm.state = START;

    unsigned char bcc2 = buffer[0];

    for (int i = 1; i < bufSize; i++) {
        bcc2 ^= buffer[i];
    }

    int illegal_counter = 0;
    for (int i = 0; i < bufSize; i++) {
        if (buffer[i] == ESCAPE || buffer[i] == SF_FLAG) {
            illegal_counter++;
        }
    }

    int stuffed_size = bufSize + illegal_counter + DATALESS_INFO_FRAME_SIZE;
    unsigned char *stuffed = calloc(stuffed_size, 1);

    illegal_counter = 0;
    for (int i = 0; i < bufSize; i++) {
        if (buffer[i] == ESCAPE || buffer[i] == SF_FLAG) {
            stuffed[i + illegal_counter + INFO_FRAME_DATA_OFFSET] = ESCAPE;
            stuffed[i + illegal_counter + INFO_FRAME_DATA_OFFSET + 1] = STUFF(buffer[i]);
            illegal_counter++;
        } else {
            stuffed[i + illegal_counter + INFO_FRAME_DATA_OFFSET] = buffer[i];
        }
    }

    stuffed[stuffed_size - 2] = bcc2;

    (void)build_frame(INFO, stuffed, stuffed_size);

    if (handle_write(write(fd, stuffed, stuffed_size))) {
        alarm(0);
        free(stuffed);
        return -1;
    }

    (void)signal(SIGALRM, alarmHandler);

    unsigned char expected = ((info_frame_counter - 1) % 2 == 0) ? RR1_CONTROL : RR0_CONTROL;
    
    int err;

    while (alarmCount < ll.nRetransmissions) {
        unsigned char packet;
        err = handle_read(read(fd, &packet, 1));
        if (err) break;
        sm.transition[sm.state](packet);

        if (sm.state == STOP) {
            sm.state = START;
            if (rcv_control == expected) break;
            if (rcv_control != (((info_frame_counter - 1) % 2 == 0) ? REJ0_CONTROL : REJ1_CONTROL)) continue;
            alarm(0);
            alarmCount = 0;
            alarmEnabled = FALSE;
            printf("[INFO] Frame rejected. Resending.\n");
            err = handle_write(write(fd, stuffed, stuffed_size));
            if (err) break;
        }

        if (alarmEnabled) continue;
        if (alarmCount > 0) {
                printf("[INFO] Resending I frame... (Tries left: %d)\n",
                       ll.nRetransmissions - alarmCount + 1);
                err = handle_write(write(fd, stuffed, stuffed_size));
                if (err) break;
        }
        alarm(ll.timeout);
        alarmEnabled = TRUE;
    }
    alarm(0);
    alarmCount = 0;
    alarmEnabled = FALSE;
    free(stuffed);
    if (err || rcv_control != expected) return -1;
    if (alarmCount < ll.nRetransmissions) return 0;
    printf("[INFO] Max retries reached. Timed out.\n");    
    return -1;
}

int llread(unsigned char *packet)
{
    sm.state = START;

    unsigned char data;
    int bytes;
    int counter = 0;
	
	// No longer needed (refer below)
    // alarm(ll.timeout);
    // alarmEnabled = TRUE;

    do {
        bytes = read(fd, &data, 1);
        if (handle_read(bytes)) return -1;
        
        // Wrong code in presentation
        // if (!alarmEnabled) return -1;
        
        if (bytes == 0) continue;

        int is_packet_data = (sm.state == BCC_OK) ? TRUE : FALSE;
        sm.transition[sm.state](data);

        if (is_packet_data == TRUE && sm.state == BCC_OK) {
            packet[counter++] = data;
        }
    } while (sm.state != STOP);

	// No longer needed (refer above)
    // alarm(0);
    // alarmCount = 0;
    // alarmEnabled = FALSE;

    const unsigned char bcc2 = packet[--counter];

    if (counter == 0) return 0;

    int escapes = 0;
    for (int i = 0; i < counter; i++) {
        if (packet[i] == ESCAPE) escapes++;
    }

    int destuffed_size = counter - escapes;
    unsigned char* destuffed = calloc(destuffed_size, 1);

    escapes = 0;
    for (int i = 0; i < destuffed_size; i++) {
        if (packet[i + escapes] == ESCAPE) {
            destuffed[i] = DESTUFF(packet[i + escapes + 1]);
            escapes++;
        } else {
            destuffed[i] = packet[i + escapes];
        }
    }

    unsigned char bcc2_cmp = destuffed[0];
    
    for (int i = 1; i < destuffed_size; i++) {
        bcc2_cmp ^= destuffed[i];
    }
    
    memcpy(packet, destuffed, destuffed_size);
    free(destuffed);

    unsigned char frame[SU_FRAME_SIZE];
    if (bcc2 == bcc2_cmp) {
        (void)build_frame((rcv_control == INFO0_CONTROL) ? RR1 : RR0, frame, SU_FRAME_SIZE);
        if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;

        return destuffed_size;
    } else {
        (void)build_frame((rcv_control == INFO0_CONTROL) ? REJ0 : REJ1, frame, SU_FRAME_SIZE);
        if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;

        rcv_control = 0xFF;
        return 0;
    }
}

int llclose(void)
{
    sm.state = START;
    alarmCount = 0;
    alarmEnabled = FALSE;

    (void)signal(SIGALRM, alarmHandler);

    printf("[INFO] Transmission finished. Attempting disconnection.\n");

    switch (ll.role) {
        case LlTx:
        {
            unsigned char frame[SU_FRAME_SIZE];
            (void)build_frame(DISC, frame, SU_FRAME_SIZE);
            if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;

            while (alarmCount < ll.nRetransmissions)
            {
                unsigned char packet;
                if (handle_read(read(fd, &packet, 1))) return -1;
                sm.transition[sm.state](packet);
                if (sm.state == STOP && rcv_control != DISC_CONTROL) sm.state = START;
                else if (sm.state == STOP) break;
                if (alarmEnabled) continue;
                if (alarmCount > 0) {
                    printf("[INFO] Resending DISC frame.. (Tries left: %d)\n",
                            ll.nRetransmissions - alarmCount + 1);
                    if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;
                }
                alarm(ll.timeout);
                alarmEnabled = TRUE;
            }

            alarm(0);
            free(sm.transition);

            if (alarmCount == ll.nRetransmissions) {
                printf("[INFO] Max retries reached. Disconnecting...\n");
            } else {
                (void)build_frame(UA, frame, SU_FRAME_SIZE);
                if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;
                sleep(1);
            }

            if (handle_tcsetattr(tcsetattr(fd, TCSANOW, &oldtio) == -1, FALSE)) exit(EXIT_FAILURE);

            close(fd);

            return (rcv_control == DISC_CONTROL && alarmCount < ll.nRetransmissions) ? 0 : -1;

        }
        case LlRx:
        {
            int skip = FALSE;

            alarm(ll.timeout);
            alarmEnabled = TRUE;
            
            while (!skip)
            {  
                unsigned char packet;
                if (handle_read(read(fd, &packet, 1))) return -1;
                sm.transition[sm.state](packet);

                if (alarmEnabled == FALSE) skip = TRUE;
                if (sm.state != STOP) continue;
                if (rcv_control == DISC_CONTROL) break;
                sm.state = START;
            }

            if (skip) printf("[INFO] DISC frame not received. Forcibly disconnecting...\n");

            alarm(0);
            alarmCount = 0;
            alarmEnabled = FALSE;
            sm.state = START;

            unsigned char frame[SU_FRAME_SIZE];
            (void)build_frame(DISC, frame, SU_FRAME_SIZE);
            if (handle_write(write(fd, frame, SU_FRAME_SIZE))) return -1;

            while (alarmCount < ll.nRetransmissions)
            {
                unsigned char packet;
                if (handle_read(read(fd, &packet, 1))) return -1;
                sm.transition[sm.state](packet);
                if (sm.state == STOP && rcv_control != UA_CONTROL) sm.state = START;
                else if (sm.state == STOP) break;
                if (alarmEnabled) continue;
                if (alarmCount > 0) {
                    printf("[INFO] Awaiting UA frame.. (Tries left: %d)\n",
                            ll.nRetransmissions - alarmCount + 1);
                }
                alarm(ll.timeout);
                alarmEnabled = TRUE;
            }

            alarm(0);
            free(sm.transition);
                        
            if (alarmCount == ll.nRetransmissions) {
                printf("[INFO] Max retries reached. Exiting...\n");
            }

            if (handle_tcsetattr(tcsetattr(fd, TCSANOW, &oldtio) == -1, FALSE)) exit(EXIT_FAILURE);

            close(fd);

            return (rcv_control == UA_CONTROL && alarmCount < ll.nRetransmissions) ? 0 : -1;
        }
        default:
            if (handle_tcsetattr(tcsetattr(fd, TCSANOW, &oldtio) == -1, FALSE)) exit(EXIT_FAILURE);

            close(fd);
            free(sm.transition);

            return -1;
    }
}
