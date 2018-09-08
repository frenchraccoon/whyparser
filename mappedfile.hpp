/**
 * Mapped file wrappers.
 * Class aimed to handle memory mapping of a file (read-only)
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#ifndef RX_MAPPED_FILE_HPP
#define RX_MAPPED_FILE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include <string>
#include <iostream>

#include "refstringmap.hpp"

/**
 * Read-Only mapping of a file in memory.
**/
class ReadOnlyMemoryMap {
public:
  /**
   * Create a new mapped file (read-only) in memory.
   *
   * @param filename The file path to be mapped.
  **/
  ReadOnlyMemoryMap(const char *filename): ReadOnlyMemoryMap() {
    map(filename);
  }

  /**
   * Destructor; automatically unmap the memory region.
  **/
  ~ReadOnlyMemoryMap() {
    unmap();
  }

  /**
   * Tune the current region for random or sequential read, starting from offset
   *
   * @param offset The starting offset
   * @param random If @c true, tune for random access; tune for sequential one otherwise
  **/
  void read_tune(size_t offset, bool random) const;

  /**
   * Return the region size.
   *
   * @return The region size
  **/
  size_t get_size() const {
    return size;
  }

  /**
   * Check if the current region is valid.
   *
   * @return @c true If the region is valid
  **/
  bool is_valid() const {
    return data != NULL;
  }

  /**
   * Return the error encountered during file mapping
   *
   * @return The errno number
  **/
  int get_error() const {
    return error;
  }

protected:
  // The mapped file descriptor
  int fd;

  // The mapped file size
  size_t size;

  // The mapped file data
  unsigned char *data;

  // The last error is the file could not be mapped
  int error;

protected:
  /**
   * Map the file in memory.
   *
   * @param filename The file path to be mapped.
  **/
  void map(const char *filename);

  /**
   * Unmap the memoy region.
  **/
  void unmap();

private:
  /* Default constructor, used as delegating constructor */
  ReadOnlyMemoryMap(): fd(-1), size(0), data(NULL), error(0) { }

  /* Forbidden foes */
  ReadOnlyMemoryMap(const ReadOnlyMemoryMap&) = delete;
  ReadOnlyMemoryMap& operator=(const ReadOnlyMemoryMap&) = delete;
};

#endif
