package net.meowsers.peach.utils;

public class PeachException extends RuntimeException {
    public PeachException(String message) {
        super(message);
    }
    public PeachException(String message, Throwable cause) {
        super(message, cause);
    }
    public PeachException(Throwable cause) {
        super(cause);
    }
}
