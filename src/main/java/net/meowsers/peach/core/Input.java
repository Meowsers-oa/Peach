package net.meowsers.peach.core;

import net.meowsers.peach.structures.Key;
import net.meowsers.peach.structures.MouseButton;
import org.lwjgl.glfw.GLFWKeyCallback;
import org.lwjgl.glfw.GLFWMouseButtonCallback;
import org.lwjgl.glfw.GLFWCursorPosCallback;
import org.lwjgl.glfw.GLFWScrollCallback;
import org.lwjgl.glfw.GLFWWindowFocusCallback;

import java.util.Arrays;

import static org.lwjgl.glfw.GLFW.*;

public class Input {

    private static long windowHandle = 0;
    private static boolean cursorLocked = false;

    private static final boolean[] keys = new boolean[GLFW_KEY_LAST + 1];
    private static final boolean[] keysPressed = new boolean[GLFW_KEY_LAST + 1];
    private static final boolean[] keysReleased = new boolean[GLFW_KEY_LAST + 1];

    private static final boolean[] mouseButtons = new boolean[GLFW_MOUSE_BUTTON_LAST + 1];
    private static final boolean[] mouseButtonsPressed = new boolean[GLFW_MOUSE_BUTTON_LAST + 1];
    private static final boolean[] mouseButtonsReleased = new boolean[GLFW_MOUSE_BUTTON_LAST + 1];

    private static double mouseX = 0.0;
    private static double mouseY = 0.0;
    private static double mouseScrollX = 0.0;
    private static double mouseScrollY = 0.0;

    private static GLFWKeyCallback keyCallback;
    private static GLFWMouseButtonCallback mouseButtonCallback;
    private static GLFWCursorPosCallback cursorPosCallback;
    private static GLFWScrollCallback scrollCallback;
    private static GLFWWindowFocusCallback focusCallback;

    public static void start(long window) {
        windowHandle = window;
        cursorLocked = false;
        glfwSetWindowFocusCallback(windowHandle, focusCallback = new GLFWWindowFocusCallback() {
            @Override public void invoke(long window, boolean focused) {
                if (!focused) {
                    Arrays.fill(keys, false);
                    Arrays.fill(mouseButtons, false);
                    endFrame();
                    unlockCursor();
                }
            }
        });
        glfwSetKeyCallback(windowHandle, keyCallback = new GLFWKeyCallback() {
            @Override
            public void invoke(long window, int key, int scancode, int action, int mods) {
                if (key >= 0 && key < keys.length) {
                    if (action == GLFW_PRESS) {
                        keys[key] = true;
                        keysPressed[key] = true;
                    } else if (action == GLFW_RELEASE) {
                        keys[key] = false;
                        keysReleased[key] = true;
                    } else if (action == GLFW_REPEAT) {
                        keys[key] = true;
                    }
                }
            }
        });

        glfwSetMouseButtonCallback(windowHandle, mouseButtonCallback = new GLFWMouseButtonCallback() {
            @Override
            public void invoke(long window, int button, int action, int mods) {
                if (button >= 0 && button < mouseButtons.length) {
                    if (action == GLFW_PRESS) {
                        mouseButtons[button] = true;
                        mouseButtonsPressed[button] = true;
                    } else if (action == GLFW_RELEASE) {
                        mouseButtons[button] = false;
                        mouseButtonsReleased[button] = true;
                    }
                }
            }
        });

        glfwSetCursorPosCallback(windowHandle, cursorPosCallback = new GLFWCursorPosCallback() {
            @Override
            public void invoke(long window, double xpos, double ypos) {
                mouseX = xpos;
                mouseY = ypos;
            }
        });

        glfwSetScrollCallback(windowHandle, scrollCallback = new GLFWScrollCallback() {
            @Override
            public void invoke(long window, double xoffset, double yoffset) {
                mouseScrollX = xoffset;
                mouseScrollY = yoffset;
            }
        });
    }

    /** Call at the end of every frame loop to clear single-frame states. */
    public static void endFrame() {
        Arrays.fill(keysPressed, false);
        Arrays.fill(keysReleased, false);
        Arrays.fill(mouseButtonsPressed, false);
        Arrays.fill(mouseButtonsReleased, false);
        mouseScrollX = 0.0;
        mouseScrollY = 0.0;
    }

    // --- Keyboard Actions ---

    public static boolean isKeyDown(Key key) {
        int code = key.getGlfwCode();
        return code >= 0 && code < keys.length && keys[code];
    }

    public static boolean isKeyPressed(Key key) {
        int code = key.getGlfwCode();
        return code >= 0 && code < keys.length && keysPressed[code];
    }

    public static boolean isKeyReleased(Key key) {
        int code = key.getGlfwCode();
        return code >= 0 && code < keys.length && keysReleased[code];
    }

    // --- Mouse Actions ---

    public static boolean isMouseButtonDown(MouseButton button) {
        int code = button.getGlfwCode();
        return code >= 0 && code < mouseButtons.length && mouseButtons[code];
    }

    public static boolean isMouseButtonClicked(MouseButton button) {
        int code = button.getGlfwCode();
        return code >= 0 && code < mouseButtonsPressed.length && mouseButtonsPressed[code];
    }

    public static boolean isMouseButtonReleased(MouseButton button) {
        int code = button.getGlfwCode();
        return code >= 0 && code < mouseButtonsReleased.length && mouseButtonsReleased[code];
    }

    // --- Modifiers ---

    public static boolean isShiftPressed() {
        return isKeyDown(Key.LEFT_SHIFT) || isKeyDown(Key.RIGHT_SHIFT);
    }

    public static boolean isControlPressed() {
        return isKeyDown(Key.LEFT_CONTROL) || isKeyDown(Key.RIGHT_CONTROL);
    }

    public static boolean isAltPressed() {
        return isKeyDown(Key.LEFT_ALT) || isKeyDown(Key.RIGHT_ALT);
    }

    public static boolean isSuperPressed() {
        return isKeyDown(Key.LEFT_SUPER) || isKeyDown(Key.RIGHT_SUPER);
    }

    // --- Cursor & Scroll Getters ---

    public static double getMouseX() {
        return mouseX;
    }

    public static double getMouseY() {
        return mouseY;
    }

    public static double getMouseScrollX() {
        return mouseScrollX;
    }

    public static double getMouseScrollY() {
        return mouseScrollY;
    }

    // --- Cursor Mode / Locking ---

    public static void setCursorLocked(boolean locked) {
        cursorLocked = locked;
        if (windowHandle != 0) {
            glfwSetInputMode(windowHandle, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
    }

    public static boolean isCursorLocked() {
        return cursorLocked;
    }

    public static void lockCursor() {
        setCursorLocked(true);
    }

    public static void unlockCursor() {
        setCursorLocked(false);
    }

    public static void setCursorHidden(boolean hidden) {
        if (windowHandle != 0) {
            glfwSetInputMode(windowHandle, GLFW_CURSOR, hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
        }
    }

    public static long getWindowHandle() {
        return windowHandle;
    }

    public static void cleanup() {
        if (windowHandle != 0) {
            if (cursorLocked) glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetKeyCallback(windowHandle, null);
            glfwSetMouseButtonCallback(windowHandle, null);
            glfwSetCursorPosCallback(windowHandle, null);
            glfwSetScrollCallback(windowHandle, null);
            glfwSetWindowFocusCallback(windowHandle, null);
        }
        if (keyCallback != null) keyCallback.free();
        if (mouseButtonCallback != null) mouseButtonCallback.free();
        if (cursorPosCallback != null) cursorPosCallback.free();
        if (scrollCallback != null) scrollCallback.free();
        if (focusCallback != null) focusCallback.free();
        keyCallback = null;
        mouseButtonCallback = null;
        cursorPosCallback = null;
        scrollCallback = null;
        focusCallback = null;
        windowHandle = 0;
        cursorLocked = false;
        Arrays.fill(keys, false);
        Arrays.fill(mouseButtons, false);
        mouseX = mouseY = 0;
        endFrame();
    }
}
