package net.meowsers.peach.utils;

import static org.lwjgl.glfw.GLFW.*;

public class Time {

    private static float startTime = 0.0f;
    private static float lastFrameTime = 0.0f;
    private static float deltaTime = 0.0f;
    private static float currentTime = 0.0f;

    // Zero-allocation ring buffer for frame timestamps (in milliseconds)
    private static final int HISTORY_CAPACITY = 1024;
    private static final float[] frameTimestamps = new float[HISTORY_CAPACITY];
    private static int head = 0;
    private static int size = 0;

    public static void start() {
        startTime = (float) glfwGetTime();
        lastFrameTime = startTime;
        head = 0;
        size = 0;
    }

    public static void update() {
        currentTime = (float) glfwGetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        float currentMs = currentTime * 1000.0f;

        // Push current frame timestamp to the ring buffer
        int tail = (head + size) % HISTORY_CAPACITY;
        if (size < HISTORY_CAPACITY) {
            frameTimestamps[tail] = currentMs;
            size++;
        } else {
            frameTimestamps[head] = currentMs;
            head = (head + 1) % HISTORY_CAPACITY;
        }
    }

    public static float getTimeSinceStart() {
        return currentTime - startTime;
    }

    public static float getFPS() {
        return deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    }

    /** Returns FPS sampled over the given amount of milliseconds. */
    public static float getFPS(int milis) {
        if (milis <= 0) return getFPS();

        float currentMs = currentTime * 1000.0f;
        float cutoff = currentMs - milis;

        int count = 0;
        float oldestInWindow = -1.0f;

        for (int i = 0; i < size; i++) {
            int index = (head + i) % HISTORY_CAPACITY;
            float t = frameTimestamps[index];
            if (t >= cutoff) {
                if (oldestInWindow < 0.0f) {
                    oldestInWindow = t;
                }
                count++;
            }
        }

        if (count <= 1 || oldestInWindow < 0.0f) {
            return getFPS();
        }

        float elapsedSec = (currentMs - oldestInWindow) / 1000.0f;
        return elapsedSec > 0.0f ? (count - 1) / elapsedSec : getFPS();
    }

    public static float getDeltaTime() {
        return deltaTime;
    }
}