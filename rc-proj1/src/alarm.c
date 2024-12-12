#include <stdio.h>

#include "../include/alarm.h"

int alarmEnabled = FALSE;
int alarmCount = 0;

void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
}
