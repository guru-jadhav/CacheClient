package com.gurujadhav.cacheclient;

/**
 * Exception thrown when network socket or DNS operations fail.
 */
public class NetworkException extends CacheException {
    public NetworkException(String message) {
        super(message);
    }

    public NetworkException(String message, Throwable cause) {
        super(message, cause);
    }
}
