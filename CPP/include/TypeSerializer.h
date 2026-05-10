#pragma once
#include <list>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

static const char DELIM = '\x1F';

template <typename T> struct ContainerName {
  static std::string name() { return "unknown"; }
};

template <> struct ContainerName<std::string> {
  static std::string name() { return "string"; }
};
template <typename T> struct ContainerName<std::vector<T>> {
  static std::string name() { return "vector"; }
};
template <typename T> struct ContainerName<std::list<T>> {
  static std::string name() { return "list"; }
};
template <typename T> struct ContainerName<std::set<T>> {
  static std::string name() { return "set"; }
};
template <typename T> struct ContainerName<std::multiset<T>> {
  static std::string name() { return "multiset"; }
};
template <typename T> struct ContainerName<std::unordered_set<T>> {
  static std::string name() { return "unordered_set"; }
};
template <typename T> struct ContainerName<std::unordered_multiset<T>> {
  static std::string name() { return "unordered_multiset"; }
};
template <typename T> struct ContainerName<std::queue<T>> {
  static std::string name() { return "queue"; }
};
template <typename T> struct ContainerName<std::stack<T>> {
  static std::string name() { return "stack"; }
};


template <typename T> struct TypeName {
  static std::string name() { return "unknown"; }
};
template <> struct TypeName<int> {
  static std::string name() { return "int"; }
};
template <> struct TypeName<long long> {
  static std::string name() { return "long long"; }
};
template <> struct TypeName<float> {
  static std::string name() { return "float"; }
};
template <> struct TypeName<double> {
  static std::string name() { return "double"; }
};
template <> struct TypeName<bool> {
  static std::string name() { return "bool"; }
};
template <> struct TypeName<char> {
  static std::string name() { return "char"; }
};


// Detects range-based iteration support — used to split iterable vs pop-based
// containers
template <typename T, typename = void> struct is_iterable : std::false_type {};
template <typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>()))>>
    : std::true_type {};


// Detects top() — distinguishes std::stack from std::queue in the pop-based
// path
template <typename T, typename = void> struct has_top : std::false_type {};
template <typename T>
struct has_top<T, std::void_t<decltype(std::declval<T>().top())>>
    : std::true_type {};


// Detects push_back() — distinguishes sequence containers from associative
// containers
template <typename T, typename = void>
struct has_push_back : std::false_type {};
template <typename T>
struct has_push_back<T, std::void_t<decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>>
    : std::true_type {};


// Encodes a string as "N:value" — length-prefix prevents delimiter collision
static std::string lenPrefix(const std::string &s) {
  return std::to_string(s.size()) + ':' + s;
}

// Converts a single element to string — char needs special handling
template <typename T> static std::string elementToString(const T &elem) {
  if constexpr (std::is_same_v<T, char>)
    return std::string(1, elem);
  else
    return std::to_string(elem);
}

// Converts a string token back to its original type T
template <typename T> static T stringToValue(const std::string &str) {
  if constexpr (std::is_same_v<T, char>)
    return str[0];
  else if constexpr (std::is_same_v<T, bool>)
    return str == "1";
  else if constexpr (std::is_same_v<T, int>)
    return std::stoi(str);
  else if constexpr (std::is_same_v<T, long long>)
    return std::stoll(str);
  else if constexpr (std::is_same_v<T, float>)
    return std::stof(str);
  else if constexpr (std::is_same_v<T, double>)
    return std::stod(str);
  else
    return T{};
}

/**
 * @brief Parses length-prefixed blocks from a serialized string.
 *        Format: "N:value\x1FM:value\x1F..."
 *        "6:vector\x1F3:int\x1F1:1" -> ["vector", "int", "1"]
 */
static std::vector<std::string> parseBlocks(const std::string &raw) {
  std::vector<std::string> blocks;
  size_t i = 0;

  while (i < raw.size()) {
    size_t colonPos = raw.find(':', i);
    if (colonPos == std::string::npos)
      break;

    size_t blockLen = std::stoull(raw.substr(i, colonPos - i));
    i = colonPos + 1;

    if (i + blockLen > raw.size())
      break;
    blocks.push_back(raw.substr(i, blockLen));
    i += blockLen;

    if (i < raw.size() && raw[i] == DELIM)
      i++;
  }

  return blocks;
}


class Serializer {
public:
  // Primitive wire format: "N:type\x1FM:value"  e.g. int 42 -> "3:int\x1F2:42"
  template <typename T> static std::string serializePrimitive(const T &value) {
    return lenPrefix(TypeName<T>::name()) + DELIM +
           lenPrefix(elementToString(value));
  }

  /**
   * @brief Serializes any supported STL container.
   *        Wire format: "N:container\x1FM:type\x1Flen:v1\x1Flen:v2..."
   *
   * @note  std::string is a passthrough — stored as "6:string\x1FN:value"
   * @note  queue/stack are copied before draining to avoid mutating caller's
   * data
   */
  template <typename T> static std::string serializeContainer(const T &value) {
    std::string containerName = ContainerName<T>::name();

    if constexpr (std::is_same_v<T, std::string>) {
      return lenPrefix("string") + DELIM + lenPrefix(value);
    }

    using ElementType = typename T::value_type;
    std::string result = lenPrefix(containerName) + DELIM +
                         lenPrefix(TypeName<ElementType>::name());

    if constexpr (is_iterable<T>::value) {
      for (const auto &elem : value)
        result += DELIM + lenPrefix(elementToString(elem));
    } else {
      T copy = value;
      while (!copy.empty()) {
        ElementType elem;
        if constexpr (has_top<T>::value)
          elem = copy.top();
        else
          elem = copy.front();
        copy.pop();
        result += DELIM + lenPrefix(elementToString(elem));
      }
    }

    return result;
  }

  // "3:int\x1F2:42" -> 42
  template <typename T>
  static std::optional<T> deserializePrimitive(const std::string &raw) {
    auto blocks = parseBlocks(raw);
    if (blocks.size() < 2)
      return std::nullopt;
    try {
      return stringToValue<T>(blocks[1]);
    } catch (...) {
      return std::nullopt;
    }
  }

  /**
   * @brief Deserializes a raw wire string back into container T.
   *        blocks[0] = container name, blocks[1] = element type, blocks[2+] =
   * elements
   *
   * @note  Caller must request the correct T — no runtime type checking is
   * done.
   */
  template <typename T>
  static std::optional<T> deserializeContainer(const std::string &raw) {
    auto blocks = parseBlocks(raw);
    if (blocks.size() < 2)
      return std::nullopt;

    if constexpr (std::is_same_v<T, std::string>)
      return blocks[1];

    using ElementType = typename T::value_type;
    T result;

    for (size_t i = 2; i < blocks.size(); i++) {
      try {
        ElementType elem = stringToValue<ElementType>(blocks[i]);
        if constexpr (has_top<T>::value)
          result.push(elem);
        else if constexpr (is_iterable<T>::value) {
          if constexpr (has_push_back<T>::value)
            result.push_back(elem);
          else
            result.insert(elem);
        } else {
          result.push(elem);
        }
      } catch (...) {
        return std::nullopt;
      }
    }

    return result;
  }
};