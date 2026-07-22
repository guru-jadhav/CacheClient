package com.gurujadhav.cacheclient;

/**
 * Exception thrown when parsing or deserializing database payloads fails.
 */
public class DeserializeException extends CacheException {
    public DeserializeException(String message) {
        super(message);
    }

    public DeserializeException(String message, Throwable cause) {
        super(message, cause);
    }
}
