#pragma once 

#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void SleepS(int miliseconds);