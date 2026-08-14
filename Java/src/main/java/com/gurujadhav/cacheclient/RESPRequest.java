package com.gurujadhav.cacheclient;

import java.util.Optional;

/**
 * Representation of a stateless multi-DB command request in CacheCore.
 */
public class RESPRequest {
    public String cmd;
    public int dbIndex = -1;
    public Optional<String> key = Optional.empty();
    public Optional<String> value = Optional.empty();
    public Optional<Boolean> expires = Optional.empty();
}
