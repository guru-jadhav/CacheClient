package com.gurujadhav.cacheclient;

/**
 * Base exception class for all Java CacheClient operations.
 */
public class CacheException extends RuntimeException {
    public CacheException(String message) {
        super(message);
    }

    public CacheException(String message, Throwable cause) {
        super(message, cause);
    }
}
