package com.gurujadhav.cacheclient;

import java.util.ArrayList;
import java.util.List;

/**
 * Utility class to serialize/deserialize RESP protocol wire payloads.
 */
public class RESPParser {
    private static final String CRLF = "\r\n";

    private static String lenPrefix(String s) {
        return "$" + s.length() + CRLF + s + CRLF;
    }

    private static boolean handleSimpleString(String resp) {
        return resp.contains(CRLF);
    }

    private static boolean handleError(String resp) {
        return resp.contains(CRLF);
    }

    private static boolean handleInteger(String resp) {
        return resp.contains(CRLF);
    }

    private static boolean handleBulk(String resp) {
        int crlfPos = resp.indexOf(CRLF);
        if (crlfPos == -1) return false;

        try {
            int len = Integer.parseInt(resp.substring(1, crlfPos));
            if (len == -1) return true; // Null bulk string "$-1\r\n"

            int expectedSize = crlfPos + 2 + len + 2;
            return resp.length() >= expectedSize;
        } catch (NumberFormatException e) {
            return false;
        }
    }

    /**
     * Checks if the received buffer contains a complete RESP frame.
     * @return boolean - true if a full frame boundary has been reached
     */
    public static boolean isComplete(String rawResponse) {
        if (rawResponse == null || rawResponse.isEmpty()) {
            return false;
        }

        char firstChar = rawResponse.charAt(0);
        switch (firstChar) {
            case '+':
                return handleSimpleString(rawResponse);
            case '-':
                return handleError(rawResponse);
            case ':':
                return handleInteger(rawResponse);
            case '$':
                return handleBulk(rawResponse);
            default:
                return false;
        }
    }

    /**
     * Encodes a RESPRequest into a RESP protocol array wire representation.
     * @return String - Encoded request string
     */
    public static String encode(RESPRequest req) {
        List<String> elements = new ArrayList<>();

        elements.add(String.valueOf(req.dbIndex));
        elements.add(req.cmd);

        req.key.ifPresent(elements::add);
        req.value.ifPresent(elements::add);
        req.expires.ifPresent(exp -> elements.add(exp ? "1" : "0"));

        StringBuilder encoded = new StringBuilder();
        encoded.append("*").append(elements.size()).append(CRLF);
        for (String element : elements) {
            encoded.append(lenPrefix(element));
        }

        return encoded.toString();
    }

    /**
     * Decodes a raw RESP response string into a RESPResponse object.
     * @return RESPResponse - The parsed structure
     */
    public static RESPResponse decode(String rawResponse) {
        if (rawResponse == null || rawResponse.isEmpty()) {
            throw new DeserializeException("empty response from server");
        }

        RESPResponse result = new RESPResponse();
        int crlfPos = rawResponse.indexOf(CRLF);

        if (crlfPos == -1) {
            throw new DeserializeException("malformed RESP response — no CRLF found");
        }

        char typeChar = rawResponse.charAt(0);
        switch (typeChar) {
            case '+': // Simple String
                result.value = rawResponse.substring(1, crlfPos);
                break;
            case '-': // Error String
                result.isError = true;
                result.value = rawResponse.substring(1, crlfPos);
                break;
            case ':': // Integer
                result.value = rawResponse.substring(1, crlfPos);
                break;
            case '$': // Bulk String
                try {
                    int len = Integer.parseInt(rawResponse.substring(1, crlfPos));
                    if (len == -1) {
                        result.isNull = true;
                    } else {
                        // Extract content after CRLF up to length
                        result.value = rawResponse.substring(crlfPos + 2, crlfPos + 2 + len);
                    }
                } catch (IndexOutOfBoundsException | NumberFormatException e) {
                    throw new DeserializeException("failed to parse bulk string content", e);
                }
                break;
            default:
                throw new DeserializeException("unknown RESP type: " + typeChar);
        }

        return result;
    }
}
