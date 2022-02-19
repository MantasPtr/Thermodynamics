#include "time_utils.h"
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>

double getTime()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec / 1000000;
}