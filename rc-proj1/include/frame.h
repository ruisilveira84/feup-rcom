#ifndef _FRAME_H_
#define _FRAME_H_

#define SU_FRAME_SIZE 5
#define DATALESS_INFO_FRAME_SIZE 6
#define INFO_FRAME_DATA_OFFSET 4

#define SF_FLAG         0x7E
#define SENDER_ADDRESS  0x03
#define SET_CONTROL     0x03
#define UA_CONTROL      0x07
#define RR0_CONTROL     0x05
#define RR1_CONTROL     0x85
#define REJ0_CONTROL    0x01
#define REJ1_CONTROL    0x81
#define DISC_CONTROL    0x0B
#define INFO0_CONTROL   0x00
#define INFO1_CONTROL   0x40

#define BLOCK_CHECK(c) ((0x03 ^ c))

typedef void (*_trans_func)(const unsigned char);
typedef _trans_func *TransitionTable;

void _sm_start(const unsigned char data);
void _sm_flag_rcv(const unsigned char data);
void _sm_a_rcv(const unsigned char data);
void _sm_c_rcv(const unsigned char data);
void _sm_bcc_ok(const unsigned char data);

typedef enum
{
    SET,
    UA,
    RR0,
    RR1,
    REJ0,
    REJ1,
    DISC,
    INFO
} FrameType;

typedef struct {
    enum {
        START,
        FLAG_RCV,
        A_RCV,
        C_RCV,
        BCC_OK,
        STOP
    } state;
    TransitionTable transition;
} StateMachine;

extern StateMachine sm;
extern unsigned char rcv_control;
extern int info_frame_counter;

int build_frame(FrameType type, unsigned char *buffer, int bufSize);

#endif
