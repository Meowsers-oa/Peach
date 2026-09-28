#include <Peach/Time.h>
#include <GLFW/glfw3.h>

mTime mTimeCreate(void) {
    mTime time = {0};
    mTimeReset(&time);
    return time;
}

void mTimeReset(mTime* time) {
    if (time == NULL) return;
    *time = (mTime){0};
    time->frequency = glfwGetTimerFrequency();
    if (time->frequency != 0) time->lastTick = glfwGetTimerValue();
}

void mTimeUpdate(mTime* time) {
    if (time == NULL) return;
    if (time->frequency == 0) {
        mTimeReset(time);
        return;
    }

    uint64_t now = glfwGetTimerValue();
    uint64_t ticks = now >= time->lastTick ? now - time->lastTick : 0;
    time->lastTick = now;
    time->deltaTime = (double)ticks / (double)time->frequency;
    time->elapsedTime += time->deltaTime;
    time->frameTime = time->deltaTime * 1000.0;
    time->fps = time->deltaTime > 0.0 ? 1.0 / time->deltaTime : 0.0;
    time->frameCount++;

    time->fpsElapsed += time->deltaTime;
    time->fpsFrames++;
    if (time->fpsElapsed >= 0.5) {
        time->averageFPS = (double)time->fpsFrames / time->fpsElapsed;
        time->fpsElapsed = 0.0;
        time->fpsFrames = 0;
    }
}

double mTimeNow(void) {
    uint64_t frequency = glfwGetTimerFrequency();
    if (frequency == 0) return 0.0;
    return (double)glfwGetTimerValue() / (double)frequency;
}
