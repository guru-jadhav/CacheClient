package com.gurujadhav.cacheclient;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

/**
 * Simple TCP socket client wrapper supporting blocking send and frame-aware receive.
 */
public class TCPClient {
    private final String domain;
    private final int port;
    private Socket socket;
    private InputStream inputStream;
    private OutputStream outputStream;

    public TCPClient(String domain, int port) {
        this.domain = domain;
        this.port = port;
    }

    /**
     * Resolves target address and connects to CacheCore server.
     * @return boolean - true if connection was established
     */
    public boolean connect() {
        try {
            InetAddress[] addresses = InetAddress.getAllByName(domain);
            for (InetAddress addr : addresses) {
                try {
                    socket = new Socket();
                    // Set a connection timeout of 5 seconds
                    socket.connect(new InetSocketAddress(addr, port), 5000);
                    inputStream = socket.getInputStream();
                    outputStream = socket.getOutputStream();
                    return true;
                } catch (IOException e) {
                    close();
                }
            }
        } catch (IOException e) {
            throw new NetworkException("DNS resolution failed for: " + domain, e);
        }
        throw new NetworkException("failed to connect to: " + domain + ":" + port);
    }

    /**
     * Sends raw bytes over TCP and blocks until the full RESP frame response is received.
     * @return String - Raw RESP response string
     */
    public synchronized String send(String raw) {
        if (socket == null || !socket.isConnected() || socket.isClosed()) {
            throw new NetworkException("client is not connected to CacheCore");
        }
        try {
            byte[] bytes = raw.getBytes(StandardCharsets.UTF_8);
            outputStream.write(bytes);
            outputStream.flush();
            return recv();
        } catch (IOException e) {
            throw new NetworkException("network error while sending to CacheCore", e);
        }
    }

    private String recv() {
        byte[] buffer = new byte[4096];
        StringBuilder response = new StringBuilder();

        try {
            while (true) {
                int bytesRead = inputStream.read(buffer);
                if (bytesRead == -1) {
                    throw new NetworkException("server closed the connection");
                }

                response.append(new String(buffer, 0, bytesRead, StandardCharsets.UTF_8));

                // Verify completeness of the RESP response stream using RESPParser
                if (RESPParser.isComplete(response.toString())) {
                    return response.toString();
                }
            }
        } catch (IOException e) {
            throw new NetworkException("network error while receiving from CacheCore", e);
        }
    }

    /**
     * Closes socket and stream resources.
     */
    public void close() {
        try {
            if (inputStream != null) inputStream.close();
            if (outputStream != null) outputStream.close();
            if (socket != null) socket.close();
        } catch (IOException ignored) {}
    }
}
