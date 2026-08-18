package com.gurujadhav.cacheclient;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

import java.util.*;

/**
 * End-to-end integration tests for the Java CacheClient library.
 */
public class CacheClientTest {
    private CacheClient cache;

    @BeforeEach
    public void setUp() {
        cache = new CacheClient("127.0.0.1", 6948);
        assertTrue(cache.connect(), "Failed to connect to CacheCore");
        cache.CLEAR(0);
    }

    @Test
    public void testPing() {
        String pong = cache.PING();
        assertEquals("PONG", pong);
    }

    @Test
    public void testPrimitiveSerialization() {
        // Integer
        assertTrue(cache.SET(0, "j_int", 42, false));
        Optional<Integer> valInt = cache.GET(0, "j_int", Integer.class);
        assertTrue(valInt.isPresent());
        assertEquals(42, valInt.get());

        // Double
        assertTrue(cache.SET(0, "j_double", 3.14159, false));
        Optional<Double> valDouble = cache.GET(0, "j_double", Double.class);
        assertTrue(valDouble.isPresent());
        assertEquals(3.14159, valDouble.get(), 0.00001);

        // Boolean
        assertTrue(cache.SET(0, "j_bool", true, false));
        Optional<Boolean> valBool = cache.GET(0, "j_bool", Boolean.class);
        assertTrue(valBool.isPresent());
        assertTrue(valBool.get());

        // String
        assertTrue(cache.SET(0, "j_string", "Hello CacheCore Java!", false));
        Optional<String> valString = cache.GET(0, "j_string", String.class);
        assertTrue(valString.isPresent());
        assertEquals("Hello CacheCore Java!", valString.get());
    }

    @Test
    public void testContainerSerialization() {
        // List (Vector/ArrayList mapping)
        List<Integer> list = Arrays.asList(10, 20, 30, 40);
        assertTrue(cache.SET(0, "j_list", list, false));
        Optional<List> resList = cache.GET(0, "j_list", List.class, Integer.class);
        assertTrue(resList.isPresent());
        assertEquals(list, resList.get());

        // Set
        Set<Double> set = new HashSet<>(Arrays.asList(1.1, 2.2, 3.3));
        assertTrue(cache.SET(0, "j_set", set, false));
        Optional<Set> resSet = cache.GET(0, "j_set", Set.class, Double.class);
        assertTrue(resSet.isPresent());
        assertEquals(set, resSet.get());

        // Queue
        Queue<String> queue = new LinkedList<>(Arrays.asList("apple", "banana", "cherry"));
        assertTrue(cache.SET(0, "j_queue", queue, false));
        Optional<LinkedList> resQueue = cache.GET(0, "j_queue", LinkedList.class, String.class);
        assertTrue(resQueue.isPresent());
        assertEquals(queue, resQueue.get());

        // Stack
        Stack<String> stack = new Stack<>();
        stack.push("first");
        stack.push("second");
        stack.push("third");
        assertTrue(cache.SET(0, "j_stack", stack, false));
        Optional<Stack> resStack = cache.GET(0, "j_stack", Stack.class, String.class);
        assertTrue(resStack.isPresent());
        Stack<String> actualStack = resStack.get();
        assertEquals("third", actualStack.pop());
        assertEquals("second", actualStack.pop());
        assertEquals("first", actualStack.pop());
        assertTrue(actualStack.isEmpty());
    }

    @Test
    public void testRawApisAndIncr() {
        // SETRAW / GETRAW
        assertTrue(cache.SETRAW(0, "raw_key", "500", false));
        Optional<String> val = cache.GETRAW(0, "raw_key");
        assertTrue(val.isPresent());
        assertEquals("500", val.get());

        // INCR
        long current = cache.INCR(0, "raw_key");
        assertEquals(501, current);

        Optional<String> valAfter = cache.GETRAW(0, "raw_key");
        assertTrue(valAfter.isPresent());
        assertEquals("501", valAfter.get());

        // INCR non-existent
        long newCounter = cache.INCR(0, "new_counter");
        assertEquals(1, newCounter);
    }

    @Test
    public void testExistsAndDel() {
        assertFalse(cache.EXISTS(0, "temp_key"));
        assertTrue(cache.SETRAW(0, "temp_key", "value", false));
        assertTrue(cache.EXISTS(0, "temp_key"));

        assertTrue(cache.DEL(0, "temp_key"));
        assertFalse(cache.EXISTS(0, "temp_key"));
        assertFalse(cache.GETRAW(0, "temp_key").isPresent());
    }

    @Test
    public void testExpire() {
        assertTrue(cache.SETRAW(0, "exp_key", "val", true));
        assertTrue(cache.EXPIRE(0, "exp_key", 1));
        assertTrue(cache.EXISTS(0, "exp_key"));
    }

    @Test
    public void testClear() {
        cache.CLEAR(1);
        assertTrue(cache.SETRAW(1, "k1", "v1", false));
        assertTrue(cache.SETRAW(1, "k2", "v2", false));
        assertTrue(cache.EXISTS(1, "k1"));
        assertTrue(cache.EXISTS(1, "k2"));

        assertTrue(cache.CLEAR(1));
        assertFalse(cache.EXISTS(1, "k1"));
        assertFalse(cache.EXISTS(1, "k2"));
    }

    @Test
    public void testExceptions() {
        // NetworkException
        CacheClient badCache = new CacheClient("127.0.0.1", 9999);
        assertThrows(NetworkException.class, badCache::connect);

        // DeserializeException
        assertTrue(cache.SETRAW(0, "junk", "not_serialized_at_all", false));
        assertThrows(DeserializeException.class, () -> cache.GET(0, "junk", Integer.class));
    }
}
