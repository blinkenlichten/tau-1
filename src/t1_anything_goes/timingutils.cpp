#include "timingutils.h"

namespace TimingUtils
{

/*----------------------------------------------------------------------------*/
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

double getTimeOfDay()
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
/*----------------------------------------------------------------------------*/
LapTimer::LapTimer()
{
    mLastTick = getTimeOfDay();
}
//------------------------------------------------------------------------------
double LapTimer::lapTime()
{
   double t_old = mLastTick;
   mLastTick = getTimeOfDay();
   return mLastTick - t_old;
}

double LapTimer::elapsed()
{
   return getTimeOfDay()-mLastTick;
}

//------------------------------------------------------------------------------
} //namespace TimingUtils
/*----------------------------------------------------------------------------*/
