#ifndef _PROFILING_H
#define _PROFILING_H

#include "platform.h"
#include <stdint.h>
#include <stdio.h>

#define MAX_EVENT_COUNT 20

void PROFILING_START(const char *profile_name);
void PROFILING_IGNORE(char ignore);
void PROFILING_EVENT(const char *event);
void PROFILING_EVENTARGS(const char *event, char arg1, char arg2, char arg3, char arg4);
void PROFILING_STOP(void);

#endif // _PROFILING_H