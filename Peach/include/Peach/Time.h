#ifndef PEACH_TIME_H
#define PEACH_TIME_H

#include <Peach/Common.h>

// Requires GLFW initialization. All values start at zero.
// deltaTime/elapsedTime are seconds; frameTime is milliseconds.
// fps is instantaneous; averageFPS refreshes over intervals of at least 0.5 seconds.
// averageFPS remains zero until the first interval completes.
mTime mTimeCreate(void);
// Samples the monotonic timer. Independent of glfwSetTime and system clock changes.
void mTimeUpdate(mTime* time);
// Restart the clock and clear frame statistics, e.g. after loading a scene.
void mTimeReset(mTime* time);
// Current monotonic timer value in seconds, with an unspecified origin.
// Subtract two samples for duration measurements; returns zero before GLFW init.
double mTimeNow(void);

#endif //PEACH_TIME_H
