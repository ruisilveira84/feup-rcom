#ifndef _SENDER_H_
#define _SENDER_H_

#include "../include/frame.h"

int handle_sender(const char *serialPort, int role, int baudRate,
                  int nTries, int timeout, const char *filename);

#endif
