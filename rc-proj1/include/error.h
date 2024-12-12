#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>

int handle_llopen(int err);
int handle_llread(int err);
int handle_llwrite(int err);
int handle_llclose(int err);
int handle_fopen(FILE *err);
int handle_open(int err);
int handle_read(int err);
int handle_write(int err);
int handle_tcgetattr(int err);
int handle_tcsetattr(int err, int new);

#endif