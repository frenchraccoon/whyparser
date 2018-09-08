/**
 * Time measurement.
 * Small helper class to measure elapsed time
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#ifndef RX_CHRONO_HPP
#define RX_CHRONO_HPP

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Small helper class to measure elapsed time
 **/
class ChronoTimer {
public:
  ChronoTimer(): start(get_chrono_nanoseconds())
  {
  }

  /**
   * Get a tick and reset timer.
   *
   * @return The elapsed time, in nanoseconds.
   **/
  uint64_t tick_ns() {
    const uint64_t now = get_chrono_nanoseconds();
    const uint64_t elapsed = now - start;
    start = now;

    return elapsed;
  }

  /**
   * Get a formatted tick and reset timer.
   *
   * @return The formatted string representing the elapsed time.
   **/
  std::string tick() {
    uint64_t tick = tick_ns() / UINT64_C(1000);
    // Guys, we still don't have std::string::format ? Really ?
    char buffer[64];
    if (tick < 1000) {
      if (static_cast<size_t>
          (snprintf(buffer, sizeof(buffer), "%" PRIu64 "us",
                    tick)) >= sizeof(buffer)) {
        perror("snprintf");
        abort();
      }
    } else if (tick < 1000000) {
      if (static_cast<size_t>
          (snprintf(buffer, sizeof(buffer), "%" PRIu64 ".%03ums",
                    tick / UINT64_C(1000),
                    static_cast<unsigned>(tick % 1000))) >= sizeof(buffer)) {
        perror("snprintf");
        abort();
      }
    } else {
      tick /= UINT64_C(1000);
      if (static_cast<size_t>
          (snprintf(buffer, sizeof(buffer), "%" PRIu64 ".%03us",
                    tick / UINT64_C(1000),
                    static_cast<unsigned>(tick % 1000))) >= sizeof(buffer)) {
        perror("snprintf");
        abort();
      }
    }
    return std::string(buffer);
  }

protected:
  /**
   * Get a nanoseconds reference, suitable for measuring (monotonic clock)
   *
   * @return The number of nanoseconds since an arbitrary epoch.
   **/
  static uint64_t get_chrono_nanoseconds() {
    struct timespec ts;
    
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
      perror("clock_gettime(CLOCK_MONOTONIC)");
      abort();
    }
    
    return ts.tv_sec*UINT64_C(1000000000) + ts.tv_nsec;
  }

protected:
  // Reference time, in nanoseconds (see @c get_chrono_nanoseconds)
  uint64_t start;
};

#endif
