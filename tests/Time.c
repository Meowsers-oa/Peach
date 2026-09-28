#include <Peach/Time.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); exit(1); \
} } while (0)

// A deterministic monotonic clock avoids sleeps and timing-dependent assertions.
static uint64_t tick = 100;
static uint64_t frequency = 1000;

uint64_t glfwGetTimerValue(void) {
    return tick;
}

uint64_t glfwGetTimerFrequency(void) {
    return frequency;
}

int main(void) {
    mTime time = mTimeCreate();
    CHECK(time.frameCount == 0 && time.deltaTime == 0 && time.averageFPS == 0);
    CHECK(fabs(mTimeNow() - .1) < 1e-9);
    tick += 16;
    mTimeUpdate(&time);
    CHECK(fabs(time.deltaTime - .016) < 1e-9);
    CHECK(fabs(time.frameTime - 16) < 1e-9);
    CHECK(fabs(time.fps - 62.5) < 1e-9);
    CHECK(time.frameCount == 1 && time.averageFPS == 0);
    tick += 34;
    mTimeUpdate(&time);
    CHECK(fabs(time.elapsedTime - .05) < 1e-9);
    tick += 450;
    mTimeUpdate(&time);
    CHECK(fabs(time.averageFPS - 6) < 1e-9);
    tick += 250;
    mTimeUpdate(&time);
    CHECK(fabs(time.averageFPS - 6) < 1e-9);
    tick += 250;
    mTimeUpdate(&time);
    CHECK(fabs(time.averageFPS - 4) < 1e-9);
    CHECK(fabs(time.elapsedTime - 1) < 1e-9 && time.frameCount == 5);
    mTimeUpdate(&time);
    CHECK(time.deltaTime == 0 && time.fps == 0 && isfinite(time.averageFPS));
    mTimeReset(&time);
    CHECK(time.elapsedTime == 0 && time.frameCount == 0 && time.averageFPS == 0);
    mTime other = mTimeCreate();
    tick += 100;
    mTimeUpdate(&time);
    CHECK(other.elapsedTime == 0 && other.frameCount == 0);
    CHECK(fabs(time.deltaTime - .1) < 1e-9);
    frequency = 0;
    mTimeReset(&time);
    mTimeUpdate(&time);
    CHECK(time.deltaTime == 0 && time.frameCount == 0 && mTimeNow() == 0);
    frequency = 1000;
    mTimeUpdate(&time);
    CHECK(time.frequency == 1000 && time.frameCount == 0);
    puts("Time checks passed.");
    return 0;
}
