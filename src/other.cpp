#include "../headers/other.h"

void SleepS(int miliseconds)
{
    #ifdef __WIN32
    Sleep(miliseconds);
    #else
    usleep(miliseconds * 1000);
    #endif
}