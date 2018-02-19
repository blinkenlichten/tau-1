#ifndef TIMINGUTILS_H
#define TIMINGUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/** Get current time of day starting from 00:00 in seconds.*/
double get_time_of_day();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace TimingUtils {

    class LapTimer
    {
    public:
        /** registers time of day in seconds when object spawned.*/
        LapTimer();

        /** Return elapsed time in seconds since last lapTime() call or since LapTimer
         *	object creation. Resets timer*/
        double lapTime();

        /** Get elapsed time since obejct creation or last lapTime() call.
         * Does not reset timer */
        double elapsed();

    private:
        double mLastTick;
    };
}
#endif //#ifdef __cplusplus

#endif // TIMINGUTILS_H
