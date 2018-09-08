/**
 * YCombinator Requests Objects.
 * Specialized record type to unserialize a hacker news log line
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#include "yrequest.hpp"

/**
 * Is the given character a space character (SP or TAB) ?
 *
 * @param c The character
 * @return @c true If the character is a space character
**/
static bool is_space(unsigned char c) {
  return c == ' ' || c == '\t';
}

/**
 * Return the value of an hexadecimal character.
 *
 * @param c The hexadecimal character
 * @return The value, or -1 upon error
**/
static int hex_value(unsigned char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else {
    return -1;
  }
}

bool WhyRequest::get_record(unsigned char *data, size_t size, size_t &offset) {
  /* Clear fields */
  reset();

  const bool empty = offset == size || data[offset] == '\n';

  /* First token is timestamp. We avoid strto*(), not being guaranteed that the C library function won't strlen() it... */
  for(timestamp = 0; offset < size && data[offset] >= '0' && data[offset] <= '9'; offset++) {
    timestamp *= 10;
    timestamp += data[offset] - '0';
  }

  /* Skip separator(s) */
  for(; offset < size && is_space(data[offset]) ; offset++) ;

  /* Query */
  for(query_size = 0, query = &data[offset]; offset < size && data[offset] != '\n'; offset++, query_size++) ;

  /* Ending line (offset must be placed at the beginning of next record) */
  if (offset < size) {
    offset++;
  }

  /* Valid record ? */
  valid = timestamp != 0 && query_size != 0;

  /* Return @c true if not yet EOF */
  return offset < size || !empty;
}

/**
 * Get the query (decoded)
 * Mostly for debugging purpose.
**/
void WhyRequest::get_query(std::string &str) const {
  size_t i;
  size_t esc = 0;
  int hex = 0;
  for(i = 0; i < query_size; i++) {
    const unsigned char c = query[i];

    switch(c) {
    case '+':
      str += ' ';
    break;
    case '%':
      /* Expecting two characters */
      esc = 2;

      /* Initial value */
      hex = 0;
    break;
    default:
      if (esc == 0) {
        str += (char) c;
      } else {
        const int value = hex_value(c);
        if (value != -1 && hex != -1) {
          hex *= 16;
          hex += value;
        } else {
          hex = -1;  /* Invalid value */
        }
        if (--esc == 0 && hex > 0) {  /* Refuse invalid and \0 */
          str += (char) hex;
        }
      }
    break;
    }
  }
}

/**
 * std::string operator (for debugging purpose mainly)
**/
WhyRequest::operator std::string () const {
  std::string str = std::to_string(timestamp);
  str += '\t';
  get_query(str);
  return str;
}

size_t WhyRequest::begin(unsigned char *data, size_t size, size_t offset) {
  assert(offset <= size);
  for(; offset != 0 && data[offset - 1] != '\n'; offset--) ;
  return offset;
}

size_t WhyRequest::end(unsigned char *data, size_t size, size_t offset) {
  assert(offset <= size);
  for(; offset < size && (offset == 0 || data[offset - 1] != '\n'); offset++) ;
  /* Skip \n */
  if (offset < size) {
    offset++;
  }
  return offset;
}

/* Instantiate templates */
template class MappedRecords<WhyRequest>;
