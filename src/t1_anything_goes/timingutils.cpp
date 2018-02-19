#include "timingutils.h"
#if defined(__unix) || defined(__APPLE__)
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <sys/timeb.h>
#include <time.h>
#include <windows.h>
#define snprintf _snprintf_s
#endif


extern "C"
{
double get_time_of_day()
{
#if defined(__unix) || defined(__APPLE__)
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tv.tv_sec + tv.tv_usec * 1e-6;
#elif defined(_WIN32)
    LARGE_INTEGER f;
    LARGE_INTEGER t;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t);
    return t.QuadPart/(double) f.QuadPart;
#else
    return 0;
#endif
}

}//extern "C"

//------------------------------------------------------------------------------
namespace TimingUtils
{
LapTimer::LapTimer()
{
    mLastTick = get_time_of_day();
}

double LapTimer::lapTime()
{
   double t_old = mLastTick;
   mLastTick = get_time_of_day();
   return mLastTick - t_old;
}

double LapTimer::elapsed()
{
   return get_time_of_day()-mLastTick;
}

//------------------------------------------------------------------------------
} //namespace TimingUtils
/*----------------------------------------------------------------------------*/
