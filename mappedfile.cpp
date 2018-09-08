/**
 * Mapped file wrappers.
 * Class aimed to handle memory mapping of a file (read-only)
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include <string>

#include "mappedfile.hpp"

/* ReadOnlyMemoryMap */

void ReadOnlyMemoryMap::map(const char *filename) {
  /* Sanity checks */

  assert(filename != NULL || ! "MappedRecords<T>::open: NULL filename");

  /* Open file, fetch its size */

  unmap();

  fd = open(filename, O_RDONLY | O_CLOEXEC, 0);
  if (fd == -1) {
    error = errno;
    return ;
  }

  struct stat buf;
  if (fstat(fd, &buf) != 0) {
    error = errno;
    unmap();
    return ;
  }

  /* Map the file in memory */

  size = (size_t) buf.st_size;
  if ((off_t) size != buf.st_size) {
    unmap();
    error = -EOVERFLOW;
    return ;
  }

  data = reinterpret_cast<unsigned char*>(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
  if (data == MAP_FAILED) {
    error = errno;
    unmap();
    return ;
  }
}

void ReadOnlyMemoryMap::unmap() {
  if (data != NULL && data != MAP_FAILED) {
    if (munmap(data, size) != 0) {
      perror("unexpected munmap error");
      abort();
    }
    data = NULL;
  }

  if (fd != -1) {
    if (close(fd) != 0) {
      perror("unexpected close error");
      abort();
    }
    fd = -1;
  }
}

void ReadOnlyMemoryMap::read_tune(size_t offset, bool random) const {
  assert(offset <= size);

  /* Sequential access now starts (read-ahead) */
  const long page = sysconf(_SC_PAGESIZE);
  assert(page > 0);

  /* Aligned address and length */
  const uintptr_t addr = (((uintptr_t) &data[offset]) / page)*page;
  const uintptr_t len = (((uintptr_t) &data[size] - addr + page - 1) / page)*page;
  const int ret = madvise((void*) addr, len, random ? MADV_RANDOM : MADV_SEQUENTIAL);
  assert(ret == 0);
}
