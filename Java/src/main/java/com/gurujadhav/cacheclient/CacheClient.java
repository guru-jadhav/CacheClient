package com.gurujadhav.cacheclient;

import java.util.Optional;

/**
 * Main client class providing the public CacheCore API endpoints.
 */
public class CacheClient {
    private final String domain;
    private final int port;
    private final TCPClient client;

    public CacheClient(String domain, int port) {
        this.domain = domain;
        this.port = port;
        this.client = new TCPClient(domain, port);
    }

    /**
     * Connects to the host using DNS resolution fallback loop.
     * @return boolean - true on successful connection
     */
    public boolean connect() {
        return client.connect();
    }

    /**
     * Sends a ping to verify connection health.
     * @return String - PONG on success
     */
    public String PING() {
        return null;
    }

    /**
     * Deletes a key from the database.
     * @return boolean - true if deleted
     */
    public boolean DEL(int db, String key) {
        return false;
    }

    /**
     * Checks if a key exists.
     * @return boolean - true if exists
     */
    public boolean EXISTS(int db, String key) {
        return false;
    }

    /**
     * Clears all keys in the specified database.
     * @return boolean - true on success
     */
    public boolean CLEAR(int db) {
        return false;
    }

    /**
     * Sets a TTL timeout (in seconds) on a key.
     * @return boolean - true on success
     */
    public boolean EXPIRE(int db, String key, int duration) {
        return false;
    }

    /**
     * Atomically increments the integer value of a key.
     * @return long - the value after increment
     */
    public long INCR(int db, String key) {
        return 0;
    }

    /**
     * Stores a raw, unserialized string value.
     * @return boolean - true on success
     */
    public boolean SETRAW(int db, String key, String value, boolean willExpire) {
        return false;
    }

    /**
     * Retrieves a raw, unserialized string value.
     * @return Optional - raw value, or empty if key does not exist
     */
    public Optional<String> GETRAW(int db, String key) {
        return Optional.empty();
    }

    /**
     * Serializes and stores a value in CacheCore.
     * @return boolean - true on success
     */
    public <T> boolean SET(int db, String key, T value, boolean willExpire) {
        return false;
    }

    /**
     * Retrieves a value from CacheCore and deserializes it into type T.
     * @return Optional - deserialized value, or empty if key does not exist
     */
    public <T> Optional<T> GET(int db, String key, Class<T> type) {
        return Optional.empty();
    }
}
