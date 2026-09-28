#ifndef PEACH_TIME_H
#define PEACH_TIME_H

#include <Peach/Common.h>

// Seconds for delta/elapsed, milliseconds for frameTime; averageFPS updates every 0.5s.
mTime mTimeCreate(void);
void mTimeUpdate(mTime* time);
void mTimeReset(mTime* time);
// Monotonic seconds; requires GLFW initialization.
double mTimeNow(void);

#endif //PEACH_TIME_H
