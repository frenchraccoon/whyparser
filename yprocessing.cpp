/**
 * YCombinator Main Parser Logic.
 * Specialization of mapped records parser to extract hacker news logs stats
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#include <string.h>
#include <strings.h>
#include <time.h>
#include <assert.h>

#include <limits>
#include <iostream>

#include "yprocessing.hpp"
#include "chrono.hpp"

void YParser::parse_records() {
  ChronoTimer timer;

  // Fetch approximate position if fast-seek is enabled (otherwise, 0)
  const bool find_position = fast_seek && from > jitter;
  const RecordLocation<WhyRequest> position = !find_position
    ? begin()
    : locate(WhyRequest(from - jitter));

  const std::string seek = find_position ? timer.tick() : "n/a";

  // Statistics
  size_t read = 0;
  size_t skipped = 0;
  size_t invalid = 0;

  // Jitter information
  time_t max_stamp = 0;
  time_t max_jitter = 0;

  // Scan all records, until the ending position
  for(const auto record : position) {
    const time_t stamp = record.get_timestamp();
    if (!record.is_valid()) {
      invalid++;
    } else if (stamp >= from && stamp <= to) {
      const RefString query = record.get_raw_query();
      wordMap[query]++;
      read++;
      
      /* Note max jitter, to evaluate if fast mode makes sense */
      if (stamp > max_stamp) {
        max_stamp = stamp;
      }
      if (stamp < max_stamp && max_stamp - stamp > max_jitter) {
        max_jitter = max_stamp - stamp;
      }
      
    } else if (fast_seek && stamp > to && stamp - to > jitter) {
      // Stop if reached ending (letting a jitter margin)
      break;
    } else {
      skipped++;
    }
  }

  const std::string scan = timer.tick();

  std::cerr << read << " records read in " << scan << " (seek: " << seek << ")" << ", " << skipped << " records skipped, " << invalid << " records invalid, jitter=" << max_jitter << "\n";
}

size_t YParser::get_distinct_queries() const {
  return wordMap.size();
}

std::vector<std::pair<RefString, unsigned>> YParser::get_top_queries(size_t top_queries) const {
  // Insert maximums into a min-priority queue
  RefStringPriorityQueue min_heap;
  for (const auto element : wordMap) {
    // Not enough elements yet: add element
    if (min_heap.size() < top_queries) {
      min_heap.push(element);
    }
    // New maximum: remove minimum and add element
    else if (element.second >= min_heap.top().second) {
      min_heap.pop();
      min_heap.push(element);
    }
  }

  // Extract sorted (revered) queue
  std::vector<std::pair<RefString, unsigned>> list;
  while(!min_heap.empty()) {
    list.push_back(min_heap.top());
    min_heap.pop();
  }

  // Emit sorted (revered) queue
  std::vector<std::pair<RefString, unsigned>> ordered_list;
  for(auto it = list.rbegin(); it != list.rend(); ++it) {
    const auto element = *it;
    ordered_list.push_back(element);
    min_heap.pop();
  }

  return ordered_list;
}
