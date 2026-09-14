package net.meowsers.peach.utils;

import org.lwjgl.BufferUtils;
import org.lwjgl.PointerBuffer;

import static org.lwjgl.glfw.GLFW.*;

public class Log {

    public static <T> void message(T message) {
        System.out.println(ConsoleColors.WHITE_BOLD + message + ConsoleColors.RESET);
    }

    public static <T> void warning(T message) {
        System.out.println(ConsoleColors.YELLOW_BRIGHT + message + ConsoleColors.RESET);
    }

    public static <T> void fatal(T message) {
        throw new PeachException((String) message);
    }

    public static void fatalGlfw() {
        PointerBuffer description = BufferUtils.createPointerBuffer(512);
        fatal("Glfw Error; Code: " + glfwGetError(description) + "\nDescription: " + description.getStringASCII());
    }

}
