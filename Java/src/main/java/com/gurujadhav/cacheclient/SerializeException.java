package com.gurujadhav.cacheclient;

/**
 * Exception thrown when client-side serialization of native Java types fails.
 */
public class SerializeException extends CacheException {
    public SerializeException(String message) {
        super(message);
    }

    public SerializeException(String message, Throwable cause) {
        super(message, cause);
    }
}
