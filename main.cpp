/**
 * Hacker News Logs Parser. Main program.
 * Parsing commandline arguments, calling parser to load and scan the file, output desired statistics
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <assert.h>

#include <limits>
#include <iostream>

#include "yprocessing.hpp"

#define VERSION "1.0"

// getopt options. (see man 3 getopt)
static const struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'v'},
  {"from", required_argument, 0, 'f'},
  {"to", required_argument, 0, 't'},

  {"fast-seek", optional_argument, 0, 's'},
  {"jitter", required_argument, 0, 'j'},

  {},
};
#define GETOPT_NON_OPTION_TYPE 1
#define MAX_OPT_TOKENS 3

// main program modes: distrinct queries, or top queries
enum whyparser_mode {
  whyparser_mode_unknown,
  whyparser_mode_distinct,
  whyparser_mode_top,
};

// convert a string into a enum whyparser_mode
static enum whyparser_mode whyparser_get(const char *mode) {
  if (strcasecmp(mode, "distinct") == 0)
    return whyparser_mode_distinct;
  else if (strcasecmp(mode, "top") == 0)
    return whyparser_mode_top;
  else
    return whyparser_mode_unknown;
}

// print program usage
static void usage(const char *prog) {
  std::cout
  << prog << " distinct [--from TIMESTAMP] [--to TIMESTAMP] input_file\n"
  << "\tOutput the number of distinct queries that have been done during a specific time range with this interface\n"
  << prog << " top nb_top_queries [--from TIMESTAMP] [--to TIMESTAMP] input_file\n"
  << "\tOutput the top N popular queries (one per line) that have been done during a specific time range\n";
}

/**
 * Parse a positive integer.
 *
 * @param s The string to be parsed.
 * @return The integer, or -1 upon error.
**/
static long int parse_int(const char *s) {
  char *end = NULL;
  unsigned long int value = strtoul(s, &end, 10);
  if (end != NULL && *end == '\0') {
    return static_cast<long int>(value);
  }
  return -1;
}

/** main(). **/
int main(int argc, char **argv) {
  // Non-options
  const char *tokens[MAX_OPT_TOKENS];
  size_t tokens_offs = 0;
  
  // Start timestamp
  time_t from = 0;

  // End timestamp
  time_t to = std::numeric_limits<time_t>::max();
  
  // Use binary search to locate approximate start
  bool fast_seek = true;
  
  // Number of top queries to print by default
  unsigned top_queries = 10;

  // Default jitter to 15 minutes (see design notes: queries are considered loosely sorted, with 5-minute chunks)
  time_t jitter = 900;

  // Parse args with getopt
  int c;
  int index;
  opterr = 0;
  optind = 1;
  while ((c = getopt_long(argc, argv, "-hvf:t:", long_options, &index)) != -1) {
    switch(c) {
    case 'f':
    case 't':
      {
        long int value = parse_int(optarg);
        if (value != -1) {
          time_t &dest = c == 'f' ? from : to;
          dest = value;
        } else {
          std::cout << "malformed value: " << optarg;
          return EXIT_FAILURE;
        }
      }
      break;

    case 'v':
      std::cout << VERSION "\n";
      return EXIT_SUCCESS;
      break;

    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;
      break;

    case 's':
      fast_seek = optarg != NULL ? strcasecmp(optarg, "yes") == 0 : true;
      break;

    case 'j':
      {
        long int value = parse_int(optarg);
        if (value != -1) {
          jitter = value;
        } else {
          std::cerr << "bad jitter value: " << optarg << "\n";
        }
      }
      break;

    case GETOPT_NON_OPTION_TYPE:
      assert(optarg != NULL);
      if (tokens_offs == MAX_OPT_TOKENS) {
        std::cerr << "too many arguments\n";
        return EXIT_FAILURE;
      }
      tokens[tokens_offs++] = optarg;
      break;

    default:
      usage(argv[0]);
      return EXIT_FAILURE;
      break;
    }
  }
  if (tokens_offs == 0) {
    std::cerr << "missing argument\n";
    return EXIT_FAILURE;
  }

  // Mode ?
  const enum whyparser_mode mode = whyparser_get(tokens[0]);
  if (mode == whyparser_mode_unknown) {
    std::cerr << "invalid mode '" << tokens[0] << "'\n";
    return EXIT_FAILURE;
  } else if (mode == whyparser_mode_top && tokens_offs >= 3) {
    top_queries = parse_int(tokens[1]);
  }

  // The filename is the last non-option argument
  const char *filename = tokens[tokens_offs - 1];

  // Create mapped records from the file, with WhyRequest as type object
  YParser parser(filename);
  if (!parser.is_valid()) {
    std::cerr << "could not map file: " << strerror(parser.get_error()) << "\n";
    return EXIT_FAILURE;
  }

  // Set fast-seek mode
  parser.set_fast_seek(fast_seek, jitter);

  // Set range
  if (from != 0) {
    parser.set_start(from);
  }

  if (to != std::numeric_limits<time_t>::max()) {
    parser.set_end(to);
  }

  // Process all records
  parser.parse_records();

  // And display desired stats
  switch(mode) {
  case whyparser_mode_distinct:
    std::cout << parser.get_distinct_queries() << "\n";
    break;
  case whyparser_mode_top:
    {
      // Emit sorted (revered) queue
      for(const auto element : parser.get_top_queries(top_queries)) {
        std::cout << (std::string) element.first << "\t" << element.second << "\n";
      }
    }
    break;
  default:
    abort();
    break;
  }

  // That's all, folks!
  return EXIT_SUCCESS;
}
