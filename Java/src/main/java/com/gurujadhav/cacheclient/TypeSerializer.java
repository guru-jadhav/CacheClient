package com.gurujadhav.cacheclient;

import java.util.*;

/**
 * Handles compile-time/runtime type mapping and serialization of Java primitives
 * and standard collections to match the C++ library wire format.
 */
public class TypeSerializer {
    private static final char DELIM = '\u001F'; // \x1F ASCII unit separator

    private static String lenPrefix(String s) {
        return s.length() + ":" + s;
    }

    private static List<String> parseBlocks(String raw) {
        List<String> blocks = new ArrayList<>();
        int i = 0;
        while (i < raw.length()) {
            int colonPos = raw.indexOf(':', i);
            if (colonPos == -1) break;

            int blockLen = Integer.parseInt(raw.substring(i, colonPos));
            i = colonPos + 1;

            if (i + blockLen > raw.length()) break;
            blocks.add(raw.substring(i, i + blockLen));
            i += blockLen;

            if (i < raw.length() && raw.charAt(i) == DELIM) {
                i++;
            }
        }
        return blocks;
    }

    private static String getTypeName(Class<?> clazz) {
        if (clazz == Integer.class || clazz == int.class) return "int";
        if (clazz == Long.class || clazz == long.class) return "long long";
        if (clazz == Float.class || clazz == float.class) return "float";
        if (clazz == Double.class || clazz == double.class) return "double";
        if (clazz == Boolean.class || clazz == boolean.class) return "bool";
        if (clazz == Character.class || clazz == char.class) return "char";
        if (clazz == String.class) return "string";
        throw new SerializeException("Unsupported element type: " + clazz.getName());
    }

    private static String getContainerName(Object container) {
        if (container instanceof String) return "string";
        if (container instanceof List) return "vector";
        if (container instanceof Set) return "set";
        if (container instanceof Queue) return "queue";
        if (container instanceof Stack) return "stack";
        throw new SerializeException("Unsupported container type: " + container.getClass().getName());
    }

    private static String elementToString(Object elem) {
        if (elem == null) return "";
        if (elem instanceof Boolean) {
            return (Boolean) elem ? "1" : "0";
        }
        return elem.toString();
    }

    @SuppressWarnings("unchecked")
    private static <T> T stringToValue(String str, Class<T> type) {
        if (type == Integer.class || type == int.class) {
            return (T) Integer.valueOf(str);
        }
        if (type == Long.class || type == long.class) {
            return (T) Long.valueOf(str);
        }
        if (type == Float.class || type == float.class) {
            return (T) Float.valueOf(str);
        }
        if (type == Double.class || type == double.class) {
            return (T) Double.valueOf(str);
        }
        if (type == Boolean.class || type == boolean.class) {
            return (T) Boolean.valueOf(str.equals("1") || str.equalsIgnoreCase("true"));
        }
        if (type == Character.class || type == char.class) {
            return (T) Character.valueOf(str.charAt(0));
        }
        if (type == String.class) {
            return (T) str;
        }
        throw new DeserializeException("Unsupported target type: " + type.getName());
    }

    /**
     * Serializes a primitive value to the wire format: "N:type\x1FM:value"
     */
    public static String serializePrimitive(Object value) {
        if (value == null) {
            throw new SerializeException("cannot serialize null value");
        }
        String typeName = getTypeName(value.getClass());
        return lenPrefix(typeName) + DELIM + lenPrefix(elementToString(value));
    }

    /**
     * Serializes any supported Collection or String to the wire format:
     * "N:container\x1FM:elementType\x1Flen:v1\x1Flen:v2..."
     */
    public static String serializeContainer(Object container) {
        if (container == null) {
            throw new SerializeException("cannot serialize null container");
        }

        String containerName = getContainerName(container);

        if (container instanceof String) {
            return lenPrefix("string") + DELIM + lenPrefix((String) container);
        }

        Collection<?> collection;
        if (container instanceof Stack) {
            Stack<?> stack = (Stack<?>) container;
            List<Object> list = new ArrayList<>();
            for (int i = stack.size() - 1; i >= 0; i--) {
                list.add(stack.get(i));
            }
            collection = list;
        } else {
            collection = (Collection<?>) container;
        }

        if (collection.isEmpty()) {
            throw new SerializeException("cannot serialize empty collection (missing element type)");
        }

        Object first = collection.iterator().next();
        String elementTypeName = getTypeName(first.getClass());

        StringBuilder result = new StringBuilder();
        result.append(lenPrefix(containerName)).append(DELIM).append(lenPrefix(elementTypeName));

        for (Object elem : collection) {
            result.append(DELIM).append(lenPrefix(elementToString(elem)));
        }

        return result.toString();
    }

    /**
     * Deserializes a raw wire string to primitive type T.
     */
    public static <T> T deserializePrimitive(String raw, Class<T> type) {
        List<String> blocks = parseBlocks(raw);
        if (blocks.size() < 2) {
            throw new DeserializeException("invalid primitive payload: " + raw);
        }
        try {
            return stringToValue(blocks.get(1), type);
        } catch (Exception e) {
            throw new DeserializeException("failed to deserialize primitive value", e);
        }
    }

    /**
     * Deserializes a raw wire string back into container T (List, Set, Queue, Stack, or String).
     */
    @SuppressWarnings("unchecked")
    public static <T> T deserializeContainer(String raw, Class<T> containerType, Class<?> elementType) {
        List<String> blocks = parseBlocks(raw);
        if (blocks.size() < 2) {
            throw new DeserializeException("invalid container payload: " + raw);
        }

        String containerName = blocks.get(0);
        if (containerType == String.class) {
            if (!containerName.equals("string")) {
                throw new DeserializeException("type mismatch: expected string, found " + containerName);
            }
            return (T) blocks.get(1);
        }

        Collection<Object> result;
        if (containerType == List.class || containerType == ArrayList.class) {
            result = new ArrayList<>();
        } else if (containerType == Set.class || containerType == HashSet.class) {
            result = new HashSet<>();
        } else if (containerType == Queue.class || containerType == LinkedList.class) {
            result = new LinkedList<>();
        } else if (containerType == Stack.class) {
            result = new Stack<>();
        } else {
            throw new DeserializeException("Unsupported container target class: " + containerType.getName());
        }

        for (int i = 2; i < blocks.size(); i++) {
            try {
                Object val = stringToValue(blocks.get(i), (Class<Object>) elementType);
                result.add(val);
            } catch (Exception e) {
                throw new DeserializeException("failed to deserialize element at index " + (i - 2), e);
            }
        }

        if (containerType == Stack.class) {
            Stack<Object> stack = (Stack<Object>) result;
            Stack<Object> temp = new Stack<>();
            while (!stack.isEmpty()) {
                temp.push(stack.pop());
            }
            return (T) temp;
        }

        return (T) result;
    }
}
