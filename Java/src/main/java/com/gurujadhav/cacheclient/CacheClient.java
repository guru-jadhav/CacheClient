package com.gurujadhav.cacheclient;

import java.util.Collection;
import java.util.Optional;
import java.util.Stack;

/**
 * Main client class providing the public CacheCore API endpoints.
 * Handles DNS fallback resolution, RESP command serialization, type conversion,
 * and connection resource lifecycle management.
 */
public class CacheClient implements AutoCloseable {
    private final String domain;
    private final int port;
    private final TCPClient client;

    /**
     * Instantiates a new CacheClient for the specified domain and port.
     * Note: This does not automatically open the socket connection. Call {@link #connect()} to connect.
     * 
     * @param domain the host domain name or IP address of the CacheCore server
     * @param port the port number the CacheCore server is listening on
     */
    public CacheClient(String domain, int port) {
        this.domain = domain;
        this.port = port;
        this.client = new TCPClient(domain, port);
    }

    /**
     * Resolves the host address via DNS and opens a TCP socket connection.
     * Supports multi-IP DNS resolution fallback loops.
     * 
     * @return true if the connection was established successfully
     * @throws NetworkException if DNS resolution fails or connection cannot be established
     */
    public boolean connect() {
        return client.connect();
    }

    /**
     * Sends a PING request to the server.
     * Used to check if the connection is alive.
     * 
     * @return the response from the server, typically "PONG"
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
     */
    public String PING() {
        RESPRequest req = new RESPRequest();
        req.cmd = "PING";
        req.dbIndex = 0;

        RESPResponse resp = RESPResponse.class.cast(null); // Will be overwritten
        resp = RESPParser.decode(client.send(RESPParser.encode(req)));
        if (resp.isError) {
            throw new DeserializeException(resp.value);
        }
        return resp.value;
    }

    /**
     * Deletes a key and its associated value from the specified database.
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key to delete
     * @return true if the key was deleted, false if it did not exist
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
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
     * Checks if a key exists in the specified database.
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key to inspect
     * @return true if the key exists, false otherwise
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
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
     * 
     * @param db the database index (0, 1, or 2) to clear
     * @return true if the database was cleared successfully
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
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
     * Sets a time-to-live timeout (in seconds) on a key in the database.
     * Note: CacheCore clamping rules may apply to minimum TTL.
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key to expire
     * @param duration the TTL duration in seconds
     * @return true if the expire timeout was set successfully
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
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
     * Atomically increments the integer value of a key in the database.
     * If the key does not exist, it is initialized to 1.
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key whose value to increment
     * @return the long integer value after increment
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the value cannot be parsed as an integer, or the server returns an error
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
     * Stores a raw, unserialized string value in the database.
     * The key will be subject to eviction rules by default (willExpire = true).
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key to associate the raw value with
     * @param value the raw string value to store
     * @return true if the raw value was stored successfully
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
     */
    public boolean SETRAW(int db, String key, String value) {
        return SETRAW(db, key, value, true);
    }

    /**
     * Stores a raw, unserialized string value in the database.
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key to associate the raw value with
     * @param value the raw string value to store
     * @param willExpire if true, the key will be subject to eviction rules
     * @return true if the raw value was stored successfully
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
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
     * Retrieves a raw, unserialized string value from the database.
     * 
     * @param db the database index (0, 1, or 2)
     * @param key the key to look up
     * @return an {@link Optional} containing the raw string value, or empty if the key does not exist
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if the server returns an error response
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
     * Serializes and stores a value (primitive or container) in the database.
     * The key will be subject to eviction rules by default (willExpire = true).
     * 
     * @param <T> the type of the value to store
     * @param db the database index (0, 1, or 2)
     * @param key the key to associate the serialized value with
     * @param value the value (primitive, Collection, or Stack) to serialize and store
     * @return true if the value was stored successfully
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if serialization fails or the server returns an error response
     */
    public <T> boolean SET(int db, String key, T value) {
        return SET(db, key, value, true);
    }

    /**
     * Serializes and stores a value (primitive or container) in the database.
     * 
     * @param <T> the type of the value to store
     * @param db the database index (0, 1, or 2)
     * @param key the key to associate the serialized value with
     * @param value the value (primitive, Collection, or Stack) to serialize and store
     * @param willExpire if true, the key will be subject to eviction rules
     * @return true if the value was stored successfully
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if serialization fails or the server returns an error response
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
     * Retrieves a value from the database and deserializes it into primitive type T or String.
     * 
     * @param <T> the target type for deserialization
     * @param db the database index (0, 1, or 2)
     * @param key the key to look up
     * @param type the Class of the target primitive type T or String
     * @return an {@link Optional} containing the deserialized value, or empty if the key does not exist
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if deserialization fails or the server returns an error response
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
     * Retrieves a collection/container from the database and deserializes it into containerType of elementType.
     * Supported container types: List.class, Set.class, Queue.class, Stack.class.
     * 
     * @param <T> the type of the container
     * @param db the database index (0, 1, or 2)
     * @param key the key to look up
     * @param containerType the Class of the container type T (e.g. List.class)
     * @param elementType the Class of the elements inside the container (e.g. Integer.class)
     * @return an {@link Optional} containing the deserialized collection, or empty if the key does not exist
     * @throws NetworkException if a network error occurs
     * @throws DeserializeException if deserialization fails or the server returns an error response
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
     * Implementation of {@link AutoCloseable#close()} to prevent resource and socket leaks.
     */
    @Override
    public void close() {
        client.close();
    }
}
