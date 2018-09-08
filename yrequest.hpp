/**
 * YCombinator Requests Objects.
 * Specialized record type to unserialize a hacker news log line
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#ifndef RX_YREQUEST_HPP
#define RX_YREQUEST_HPP

#include <string>
#include <iostream>

#include "records.hpp"

/**
* Specialized record type to unserialize a hacker news log line
**/
class WhyRequest {
public:
  /**
   * Default constreuctor.
   *
   * @comment Use @c get_record to fill this object
   **/
  WhyRequest(): timestamp(0), query(NULL), query_size(0)
  {
  }

  /**
   * Specialized constructor, to obtain an object suitable for comparisons.
   **/
  WhyRequest(time_t start): timestamp(start), query(NULL), query_size(0)
  {
  }

  /**
   * Check if this record is a valid record.
   *
   * @return @c true If the record is valid
  **/
  bool is_valid(void) const {
    return valid;
  }

  /**
   * Return the timestamp associated with the query.
   *
   * @return The timestamp (seconds since Epoch)
  **/
  time_t get_timestamp(void) const {
    return timestamp;
  }

  /**
   * Get the query (raw form)
   *
   * @return The RefString reference string of the query (not URI-decoded)
  **/
  RefString get_raw_query() const {
    return RefString(reinterpret_cast<const char*>(query), query_size);
  }

  /**
   * Get the query (decoded)
   *
   * @return The string of the query, URL-decoded (RFC 3986)
  **/
  void get_query(std::string &str) const;

  /**
   * std::string operator (for debugging purpose mainly)
  **/
  operator std::string () const;

  /**
   * Get the next record, and increment offset.
   * Used by MappedRecords<> template functions
  **/
  bool get_record(unsigned char *data, size_t size, size_t &offset);

  /**
   * Standard comparison operator <
   * Used by MappedRecords<> template functions
   **/
  bool operator > (const WhyRequest &other) const {
    return get_timestamp() > other.get_timestamp();
  }

  /**
   * Standard comparison operator <
   * Used by MappedRecords<> template functions
   **/
  bool operator < (const WhyRequest &other) const {
    return get_timestamp() < other.get_timestamp();
  }

public:
  // MappedRecords<> template functions
  static size_t begin(unsigned char *data, size_t size, size_t offset);
  static size_t end(unsigned char *data, size_t size, size_t offset);

protected:
  /**
   * Reset internal state.
  **/
  void reset() {
    timestamp = 0;
    query = NULL;
    query_size = 0;
    valid = false;
  }

protected:
  // Is the object valid ?
  bool valid;

  // Timestamp
  time_t timestamp;

  // The query pointer
  const unsigned char *query;

  // The string size
  size_t query_size;
};

#endif
