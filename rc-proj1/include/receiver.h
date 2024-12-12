#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <termios.h>

int handle_receiver(const char *serialPort, int role, int baudRate,
                    int nTries, int timeout, const char *filename);

#endif