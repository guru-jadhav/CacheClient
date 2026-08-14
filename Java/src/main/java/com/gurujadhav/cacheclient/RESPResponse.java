package com.gurujadhav.cacheclient;

/**
 * Representation of decoded RESP response wire frames.
 */
public class RESPResponse {
    public boolean isError = false;
    public boolean isNull = false;
    public String value = "";
}
