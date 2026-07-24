#include "../include/CacheClient.h"
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <stack>
#include <cassert>
#include <cmath>

#define RUN_TEST(test_func) \
    do { \
        std::cout << "[RUNNING] " << #test_func << "..." << std::endl; \
        try { \
            test_func(); \
            std::cout << "\033[32m[PASS]\033[0m " << #test_func << std::endl; \
        } catch (const std::exception& e) { \
            std::cerr << "\033[31m[FAIL]\033[0m " << #test_func << ": " << e.what() << std::endl; \
            std::exit(1); \
        } \
    } while (0)

// Helper to make a queue
template<typename T>
std::queue<T> make_queue(const std::vector<T>& elems) {
    std::queue<T> q;
    for (const auto& x : elems) q.push(x);
    return q;
}

// Helper to make a stack
template<typename T>
std::stack<T> make_stack(const std::vector<T>& elems) {
    std::stack<T> s;
    for (const auto& x : elems) s.push(x);
    return s;
}

void test_connection_and_ping() {
    CacheClient cache("127.0.0.1", 6948);
    assert(cache.connect() && "Failed to connect to CacheCore");
    
    std::string pong = cache.PING();
    assert(pong == "PONG" && "PING response must be PONG");
}

void test_primitive_serialization() {
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    
    // Clear DB 0 first
    cache.CLEAR(0);

    // Int
    assert(cache.SET(0, "p_int", 42, false));
    auto val_int = cache.GET<int>(0, "p_int");
    assert(val_int.has_value() && *val_int == 42);

    // Double
    assert(cache.SET(0, "p_double", 3.14159, false));
    auto val_double = cache.GET<double>(0, "p_double");
    assert(val_double.has_value() && std::abs(*val_double - 3.14159) < 0.00001);

    // Bool
    assert(cache.SET(0, "p_bool", true, false));
    auto val_bool = cache.GET<bool>(0, "p_bool");
    assert(val_bool.has_value() && *val_bool == true);

    // String
    assert(cache.SET(0, "p_string", std::string("Hello CacheCore!"), false));
    auto val_string = cache.GET<std::string>(0, "p_string");
    assert(val_string.has_value() && *val_string == "Hello CacheCore!");
}

void test_container_serialization() {
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    cache.CLEAR(0);

    // std::vector<int>
    std::vector<int> vec = {10, 20, 30, 40};
    assert(cache.SET(0, "c_vector", vec, false));
    auto res_vec = cache.GET<std::vector<int>>(0, "c_vector");
    assert(res_vec.has_value() && *res_vec == vec);

    // std::list<std::string>
    std::list<std::string> lst = {"apple", "banana", "cherry"};
    assert(cache.SET(0, "c_list", lst, false));
    auto res_lst = cache.GET<std::list<std::string>>(0, "c_list");
    assert(res_lst.has_value() && *res_lst == lst);

    // std::set<double>
    std::set<double> st = {1.1, 2.2, 3.3};
    assert(cache.SET(0, "c_set", st, false));
    auto res_st = cache.GET<std::set<double>>(0, "c_set");
    assert(res_st.has_value() && *res_st == st);

    // std::queue<int>
    std::queue<int> q = make_queue<int>({1, 2, 3});
    assert(cache.SET(0, "c_queue", q, false));
    auto res_q = cache.GET<std::queue<int>>(0, "c_queue");
    assert(res_q.has_value());
    // verify queue elements
    auto temp_q = *res_q;
    assert(temp_q.front() == 1); temp_q.pop();
    assert(temp_q.front() == 2); temp_q.pop();
    assert(temp_q.front() == 3); temp_q.pop();
    assert(temp_q.empty());

    // std::stack<std::string>
    std::stack<std::string> s = make_stack<std::string>({"first", "second", "third"});
    assert(cache.SET(0, "c_stack", s, false));
    auto res_s = cache.GET<std::stack<std::string>>(0, "c_stack");
    assert(res_s.has_value());
    auto temp_s = *res_s;
    assert(temp_s.top() == "third"); temp_s.pop();
    assert(temp_s.top() == "second"); temp_s.pop();
    assert(temp_s.top() == "first"); temp_s.pop();
    assert(temp_s.empty());
}

void test_raw_apis_and_incr() {
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    cache.CLEAR(0);

    // SETRAW and GETRAW
    assert(cache.SETRAW(0, "raw_key", "100", false));
    auto raw_val = cache.GETRAW(0, "raw_key");
    assert(raw_val.has_value() && *raw_val == "100");

    // INCR (must increment "100" to 101)
    long long current = cache.INCR(0, "raw_key");
    assert(current == 101);
    
    // GETRAW again to verify it is stored as "101"
    raw_val = cache.GETRAW(0, "raw_key");
    assert(raw_val.has_value() && *raw_val == "101");

    // INCR on non-existing key (should initialize to 1)
    long long new_val = cache.INCR(0, "new_counter");
    assert(new_val == 1);
    raw_val = cache.GETRAW(0, "new_counter");
    assert(raw_val.has_value() && *raw_val == "1");
}

void test_exists_and_del() {
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    cache.CLEAR(0);

    assert(!cache.EXISTS(0, "temp_key"));
    assert(cache.SETRAW(0, "temp_key", "some_value", false));
    assert(cache.EXISTS(0, "temp_key"));

    assert(cache.DEL(0, "temp_key"));
    assert(!cache.EXISTS(0, "temp_key"));
    assert(!cache.GETRAW(0, "temp_key").has_value());
}

void test_expire() {
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    cache.CLEAR(0);

    // Write a key with expiry
    assert(cache.SETRAW(0, "exp_key", "temporary", true)); 
    
    // Set TTL. Note: CacheCore clamps the minimum TTL to std::max(60, duration),
    // so this key will remain alive for 60 seconds.
    assert(cache.EXPIRE(0, "exp_key", 1)); 

    // Verify key exists immediately
    assert(cache.EXISTS(0, "exp_key"));
}

void test_clear() {
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    
    // Clear DB 1
    cache.CLEAR(1);
    
    assert(cache.SETRAW(1, "key1", "val1", false));
    assert(cache.SETRAW(1, "key2", "val2", false));
    
    assert(cache.EXISTS(1, "key1"));
    assert(cache.EXISTS(1, "key2"));

    assert(cache.CLEAR(1));

    assert(!cache.EXISTS(1, "key1"));
    assert(!cache.EXISTS(1, "key2"));
}

void test_exceptions() {
    // Port with no server
    CacheClient bad_cache("127.0.0.1", 9999);
    bool threw_network = false;
    try {
        bad_cache.connect();
    } catch (const NetworkException& e) {
        threw_network = true;
    }
    assert(threw_network && "Connecting to down port should throw NetworkException");

    // Invalid deserialization
    CacheClient cache("127.0.0.1", 6948);
    cache.connect();
    cache.CLEAR(0);

    // Store raw junk
    assert(cache.SETRAW(0, "junk_key", "not_serialized_at_all", false));
    bool threw_deserialize = false;
    try {
        cache.GET<int>(0, "junk_key");
    } catch (const DeserializeException& e) {
        threw_deserialize = true;
    }
    assert(threw_deserialize && "Getting raw junk as typed int should throw DeserializeException");
}

int main() {
    std::cout << "\033[36m=== STARTING E2E INTEGRATION TESTS FOR CACHECLIENT ===\033[0m" << std::endl;
    RUN_TEST(test_connection_and_ping);
    RUN_TEST(test_primitive_serialization);
    RUN_TEST(test_container_serialization);
    RUN_TEST(test_raw_apis_and_incr);
    RUN_TEST(test_exists_and_del);
    RUN_TEST(test_expire);
    RUN_TEST(test_clear);
    RUN_TEST(test_exceptions);
    std::cout << "\033[32m=== ALL E2E INTEGRATION TESTS PASSED SUCCESSFULLY ===\033[0m" << std::endl;
    return 0;
}
