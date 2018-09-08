/**
 * Hacker News Logs Parser. Reference string unordered map/queues.
 * Represent a string, with outer buffer pointing to an external const reference
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#ifndef RX_REFSTRING_MAP_HPP
#define RX_REFSTRING_MAP_HPP

#include <string.h>
#include <unordered_map>
#include <queue>

/**
 * Simple FNV1-a hash function
 * See <https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function>
 **/
static uint64_t fnv1a_hash(const unsigned char *str, size_t size) {
  uint64_t hash = 0xcbf29ce484222325;
  for(size_t i = 0; i < size; i++) {
    hash ^= str[i];
    hash *= 1099511628211;
  }
  return hash;
}

#if 1
/**
 * Simple FNV1-a hash function, optimized (source should ideally be aligned to 64-bit)
 * Only aimed for platforms where unaligned access is tolerated (typically x86*)
 * Note that this is actually a visible optimization, reducing overall program time by ~30%.
 * Ideally, this function should be improved to read aligned access only
 * See <https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function>
 **/
static uint64_t fnv1a_hash_fast(const unsigned char *str, size_t size) {
  // Strictly speaking, this is an aliasing violation
  const uint64_t *longs = reinterpret_cast<const uint64_t*>(str);
  const size_t size64 = size / 8;
  uint64_t hash = 0xcbf29ce484222325;
  for(size_t i = 0; i < size64; i++) {
    hash ^= longs[i];
    hash *= 1099511628211;
  }
  // Remaining bytes
  const size_t remain = size - size64*8;
  if (remain != 0) {
    hash ^= fnv1a_hash(&str[size64*8], remain);
    hash *= 1099511628211;
  }
  return hash;
}
#define fnv1a_hash fnv1a_hash_fast
#endif

/**
 * A reference string (a const char*, and a length)
 * The goal is to avoid storing the string twice in memory, and reduce allocator pressure.
**/
class RefString {
public:
  /** Default constructor **/
  RefString(): str(NULL), len(0)
  {
  }

  /**
   * Create a new reference string object.
   *
   * @param str The string pointer
   * @param len The string length
  **/
  RefString(const char *str, size_t len): str(str), len(len)
  {
  }

  /**
   * Create a new reference string object.
   *
   * @param str The string pointer
   * @param len The string length
  **/
  RefString(const char *str): str(str), len(strlen(str))
  {
  }

  /**
   * Equality operator.
  **/
  bool operator==(const RefString &other) const {
    return len == other.len && strncmp(str, other.str, len) == 0;
  }

  /**
   * Hash this object. The hash is suitable for hashtable handling.
  **/
  size_t hash() const {
    /* Use FNV1-a. We probably could use a 128-bit version, XOR-folded, for better results. */
    return static_cast<size_t>(fnv1a_hash(reinterpret_cast<const unsigned char*>(str), len));
  }

  /**
   * Append the bytes in an existing string.
  **/
  void get(std::string &s) const {
    s.append(str, len);
  }

  /**
   * Get a reference to string data.
  **/
  void get(const char* &str, size_t &len) const {
    str = this->str;
    len = this->len;
  }

  /**
   * std::string operator (for debugging purpose mainly)
  **/
  operator std::string () const {
    return std::string(str, len);
  }

  /* Note: default copy constructor et al. are fine here. */

public:
  // String external reference. We are not responsaible for the life cycle
  const char *str;

  // String length (if 0, 'str' is meaningless)
  size_t len;
};

/** Hash for RefString class. **/
struct RefStringHash
{
  std::size_t operator()(RefString const& s) const noexcept
  {
    return s.hash();
  }
};

/** Comparison for RefString class. **/
struct RefStringCompare
{
  bool operator()(const RefString& lhs, const RefString& rhs) const {
    return lhs == rhs;
  }
};

/**
 * A reference string unordered map.
 * The provided T shall be an integer numerical type.
**/
template<typename T>
class RefStringUnorderedHashMap: public std::unordered_map<RefString, T, RefStringHash>
{
};

/** A pair of RefString, and the number of hits. **/
typedef std::pair<RefString, unsigned> RefStringPriorityPair;

/** Comparison for std::priority_queue<std::pair<RefString, unsigned> class. **/
struct RefStringPriorityPairCompare
{
  bool operator()(const RefStringPriorityPair& lhs, const RefStringPriorityPair& rhs) const {
    return lhs.second > rhs.second;
  }
};

/** A pair of RefStringPriorityPair, and the number of hits. **/
typedef std::priority_queue<RefStringPriorityPair, std::vector<RefStringPriorityPair>, RefStringPriorityPairCompare> RefStringPriorityQueue;

#endif
