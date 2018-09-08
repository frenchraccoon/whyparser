/**
 * YCombinator Main Parser Logic.
 * Specialization of mapped records parser to extract hacker news logs stats
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#ifndef RX_YPROCESSING_HPP
#define RX_YPROCESSING_HPP

#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <assert.h>

#include <limits>
#include <iostream>

#include "yrequest.hpp"
#include "refstringmap.hpp"

/**
 * Specialization of mapped records parser to extract hacker news logs stats
 **/
class YParser: protected MappedRecords<WhyRequest> {
public:
  /**
   * Constructor.
   *
   * @param filename The path of the record fiel to open.
   **/
  YParser(const char* filename):
    MappedRecords<WhyRequest>(filename),
    wordMap(),
    from(0),
    to(std::numeric_limits<time_t>::max()),
    fast_seek(true),
    jitter(900)
  {
  }

  /**
   * Was the file correctly opened ?
   *
   * @return @c true If the file was opened successfully
   **/
  bool is_valid() const {
    return MappedRecords<WhyRequest>::is_valid();
  }

  /**
   * Get the last error number encountered if the file could not be opened.
   **/
  int get_error() const {
    return MappedRecords<WhyRequest>::get_error();
  }

  /**
   * Enable or disable fast file seek (using binary search)
   *
   * @param enabled If @c true, enable fast seek
   * @param jitter_s The jitter time, in seconds, to locate the start point
   * @comment This function can only be called before @c parse_records
   **/
  void set_fast_seek(bool enabled, time_t jitter_s = 900) {
    fast_seek = enabled;
    jitter = jitter_s;
  }

  /**
   * Set the range start
   *
   * @param timestamp The start range (seconds since Epoch)
   * @comment This function can only be called before @c parse_records
   **/
  void set_start(time_t timestamp) {
    from = timestamp;
  }

  /**
   * Set the range end
   *
   * @param timestamp The ending range (seconds since Epoch)
   * @comment This function can only be called before @c parse_records
   **/
  void set_end(time_t timestamp) {
    to = timestamp;
  }

  /**
   * Parse all requested records. The @c set_start, @c set_end, and
   * @c set_fast_seek function must not be called afterwards.
   **/
  void parse_records();

  /**
   * Get the number of distinct queries.
   *
   * @comment This function can only be called after @c parse_records
   **/
  size_t get_distinct_queries() const;

  /**
   * Get the top queries.
   *
   * @param top_queries The maximum number of top queries to retreive
   * @return The list of top queries, sorted in descending order
   * @comment This function can only be called after @c parse_records
   **/
  std::vector<std::pair<RefString, unsigned>> get_top_queries(size_t top_queries = 10) const;

protected:
  // hashmap (unordered map) of RefString (ie. small string objects
  // referencing mapped memory bytes) to count unique queries
  RefStringUnorderedHashMap<unsigned> wordMap;

  // Start timestamp
  time_t from;

  // End timestamp
  time_t to;

  // Enable fast-seek
  bool fast_seek;

  // Jitter for loosely ordered file
  time_t jitter;
};

#endif
