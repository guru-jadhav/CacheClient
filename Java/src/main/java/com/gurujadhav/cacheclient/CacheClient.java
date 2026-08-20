package com.gurujadhav.cacheclient;

import java.util.Collection;
import java.util.Optional;
import java.util.Stack;

/**
 * Main client class providing the public CacheCore API endpoints.
 */
public class CacheClient implements AutoCloseable {
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
        RESPRequest req = new RESPRequest();
        req.cmd = "PING";
        req.dbIndex = 0;

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return resp.value;
    }

    /**
     * Deletes a key from the database.
     * @return boolean - true if deleted
     */
    public boolean DEL(int db, String key) {
        RESPRequest req = new RESPRequest();
        req.cmd = "DEL";
        req.dbIndex = db;
        req.key = Optional.of(key);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return resp.value.equals("OK") || resp.value.equals("1");
    }

    /**
     * Checks if a key exists.
     * @return boolean - true if exists
     */
    public boolean EXISTS(int db, String key) {
        RESPRequest req = new RESPRequest();
        req.cmd = "EXISTS";
        req.dbIndex = db;
        req.key = Optional.of(key);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return resp.value.equals("1");
    }

    /**
     * Clears all keys in the specified database.
     * @return boolean - true on success
     */
    public boolean CLEAR(int db) {
        RESPRequest req = new RESPRequest();
        req.cmd = "CLEAR";
        req.dbIndex = db;

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return true;
    }

    /**
     * Sets a TTL timeout (in seconds) on a key.
     * @return boolean - true on success
     */
    public boolean EXPIRE(int db, String key, int duration) {
        RESPRequest req = new RESPRequest();
        req.cmd = "EXPIRE";
        req.dbIndex = db;
        req.key = Optional.of(key);
        req.value = Optional.of(String.valueOf(duration));

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return true;
    }

    /**
     * Atomically increments the integer value of a key.
     * @return long - the value after increment
     */
    public long INCR(int db, String key) {
        RESPRequest req = new RESPRequest();
        req.cmd = "INCR";
        req.dbIndex = db;
        req.key = Optional.of(key);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        try {
            return Long.parseLong(resp.value);
        } catch (NumberFormatException e) {
            throw new DeserializeException("failed to parse incremented value: " + resp.value, e);
        }
    }

    /**
     * Stores a raw, unserialized string value.
     * @return boolean - true on success
     */
    public boolean SETRAW(int db, String key, String value, boolean willExpire) {
        RESPRequest req = new RESPRequest();
        req.cmd = "SET";
        req.dbIndex = db;
        req.key = Optional.of(key);
        req.value = Optional.of(value);
        req.expires = Optional.of(willExpire);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return resp.value.equals("1");
    }

    /**
     * Retrieves a raw, unserialized string value.
     * @return Optional - raw value, or empty if key does not exist
     */
    public Optional<String> GETRAW(int db, String key) {
        RESPRequest req = new RESPRequest();
        req.cmd = "GET";
        req.dbIndex = db;
        req.key = Optional.of(key);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        if (resp.isNull) {
            return Optional.empty();
        }
        return Optional.of(resp.value);
    }

    /**
     * Serializes and stores a value in CacheCore.
     * @return boolean - true on success
     */
    public <T> boolean SET(int db, String key, T value, boolean willExpire) {
        String formatted;
        if (value instanceof String || value instanceof Collection || value instanceof Stack) {
            formatted = TypeSerializer.serializeContainer(value);
        } else {
            formatted = TypeSerializer.serializePrimitive(value);
        }

        RESPRequest req = new RESPRequest();
        req.cmd = "SET";
        req.dbIndex = db;
        req.key = Optional.of(key);
        req.value = Optional.of(formatted);
        req.expires = Optional.of(willExpire);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return resp.value.equals("1");
    }

    /**
     * Retrieves a value from CacheCore and deserializes it into primitive type T or String.
     * @return Optional - deserialized value, or empty if key does not exist
     */
    public <T> Optional<T> GET(int db, String key, Class<T> type) {
        RESPRequest req = new RESPRequest();
        req.cmd = "GET";
        req.dbIndex = db;
        req.key = Optional.of(key);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        if (resp.isNull) {
            return Optional.empty();
        }

        if (type == String.class) {
            return Optional.of(type.cast(TypeSerializer.deserializeContainer(resp.value, String.class, null)));
        } else {
            return Optional.of(TypeSerializer.deserializePrimitive(resp.value, type));
        }
    }

    /**
     * Retrieves a collection from CacheCore and deserializes it into containerType of elementType.
     * @return Optional - deserialized collection, or empty if key does not exist
     */
    public <T> Optional<T> GET(int db, String key, Class<T> containerType, Class<?> elementType) {
        RESPRequest req = new RESPRequest();
        req.cmd = "GET";
        req.dbIndex = db;
        req.key = Optional.of(key);

        RESPResponse resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        if (resp.isNull) {
            return Optional.empty();
        }

        return Optional.of(TypeSerializer.deserializeContainer(resp.value, containerType, elementType));
    }

    /**
     * Closes the underlying socket and network resources.
     */
    @Override
    public void close() {
        client.close();
    }
}
