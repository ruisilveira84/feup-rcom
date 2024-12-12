#ifndef _ALARM_H_
#define _ALARM_H_

#define FALSE 0
#define TRUE 1

extern int alarmEnabled;
extern int alarmCount;

// Alarm function handler
void alarmHandler(int signal);

#endif 
