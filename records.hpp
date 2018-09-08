/**
 * Records reader.
 * An abstract generic "record" reader on top of a mapped file
 * Copyright (C) 2018 Xavier Roche (http://www.httrack.com/)
 * All rights reserved.
 * License: http://opensource.org/licenses/BSD-2-Clause
 **/

#ifndef RX_RECORDS_HPP
#define RX_RECORDS_HPP

#include <string>
#include <functional>
#include <iostream>

#include "mappedfile.hpp"

/** Forward declaration **/
template <typename T>
class RecordLocation;

/**
 * Read-Only mapping of a set of records within a file in memory.
 * Records can be variable-sized, as long as seeking the previous/next record is feasible on an arbitrary position.
 *
 * The provided type must implement the following non-static function:
 *
 * bool get_record(unsigned char *data, size_t size, size_t &offset);
 *   Fills the current object with the the current record, and increment offset to the next one.
 *
 * And the following static functions:
 *
 * static size_t begin(unsigned char *data, size_t size, size_t offset);
 *   Request the file offset location of the beginning of the record that contains the provided position
 * static size_t end(unsigned char *data, size_t size, size_t offset);
 *   Request the next file offset location of the beginning of the record that contains the provided position
 *
 * And should also implement the following comparison functions:
 *
 * bool operator > (const T &other) const
 * bool operator < (const T &other) const
**/
template <typename T>
class MappedRecords: public ReadOnlyMemoryMap {
public:
  /**
   * Create a new mapped file (read-only) in memory.
   *
   * @param filename The file path to be mapped.
  **/
  MappedRecords(const char *filename): ReadOnlyMemoryMap(filename)
  {
  }

  /**
   * Destructor; automatically unmap the memory region.
  **/
  ~MappedRecords()
  {
  }

  /**
   * Read the next record on the memory region
   *
   * @param record The record buffer
   * @return @c true If the record has been successfully read (it. not at end of file)
   **/
  bool get_record(T &record, size_t &offset) const {
    return record.get_record(data, size, offset);
  }

  /**
   * Locate a specific record in a sorted set of records.
   *
   * @param compare The comparison function, taking a record to compare to the desired target.
   * @return The nearest offset found, aligned to the beginning of a record (or equal to the region size)
  **/
  RecordLocation<T> locate(std::function<int(const T &record)> compare) const {
    /* Random access (no aggressive read-ahead) */
    ReadOnlyMemoryMap::read_tune(0, true);

    /* Find position and return a RecordLocation<> */
    T record;
    return RecordLocation<T>(*this, locate(compare, record, 0, size));
  }

  /**
   * Locate a specific record in a sorted set of records, using T
   * comparators.
   *
   * @param element The reference element (must implement
   * 'bool operator > (const T &other) const'
   * and 'bool operator < (const T &other) const').
   * @return The nearest offset found, aligned to the beginning of a record (or equal to the region size)
  **/
  RecordLocation<T> locate(const T &reference) const {
    return locate([reference](const T &record) {
        if (record > reference) {
          return 1;
        } else if (record < reference) {
          return -1;
        } else {
          return 0;
        }
      });
  }

  /**
   * Return the location of the beginning of file.
  **/
  RecordLocation<T> begin() const {
    return RecordLocation<T>(*this, 0);
  }

protected:
  /**
   * Locate a specific record in a sorted set of records, using a binary search.
   *
   * @param compare The comparison function, taking a record to compare to the desired target.
   * @param record The record buffer to be used.
   * @param left The left position, which shall start on a record frontier.
   * @param right The right position, which shall start on a record frontier (or be equal to region size).
   * @return The nearest offset found, aligned to the beginning of a record (or equal to the region size)
  **/
  size_t locate(std::function<int(const T &record)> compare, T &record, size_t left, size_t right) const {
    /* Left is inclusive, right is exclusive. Offsets points on the beginning of next record */
    /* Ending position (== size) is the last offset (out-of-range) */

    /* Note: binary search ; all non-exiting pathes must satisfy
       non-invariant, progressive behavior */
    for(;;) {
      /* Empty range */
      if (left == right) {
        return left;
      }
      /* From this point we *may* have a middle element
         (but left == right is possible); seek it on a record frontier */

      /* Verify invariants */
      assert(left < size);
      assert(right <= size);
      assert(left < right);

      const size_t middle = begin(( left + right ) / 2);
      assert(middle >= left && middle <= right);

      /* Compare middle element */
      size_t position = middle;
      get_record(record, position);

      /* After this record, position has been set to next record */
      const int cmp = compare(record);

      /* Left side */
      if (cmp > 0) {
        assert(right > middle); /* non invariant */
        right = middle; /* exclusive */
      }
      /* Right side */
      else if (cmp < 0) {
        assert(left < middle + 1); /* non invariant */
        left = position; /* inclusive */
      }
      /* Middle */
      else {
        return middle;
      }
    }
    assert(! "We should not be there, I'm afraid");
    return 0;
  }

  /**
   * Return the record start spanning up to 'offset'
   *
   * @param offset An arbitrary offset within the file which is part of a record
  **/
  size_t begin(size_t offset) const {
    assert(offset <= size);
    return T::begin(data, size, offset);
  }

  /**
   * Return the record following 'offset'
   *
   * @param offset An arbitrary offset within the file which is part of a record
  **/
  size_t end(size_t offset) const {
    assert(offset <= size);
    return T::end(data, size, offset);
  }

private:
  /* Forbidden foes */
  MappedRecords() = delete;
  MappedRecords(const MappedRecords&) = delete;
  MappedRecords& operator=(const MappedRecords&) = delete;
};

/** Forward declaration **/
template <typename T>
class RecordIterator;

/**
 * Record location
**/
template <typename T>
class RecordLocation {
public:
  RecordLocation(const MappedRecords<T> &map, size_t offset): map(map), offset(offset), size(map.get_size())
  {
  }

  /** Standard iterator begin(). **/
  RecordIterator<T> begin() const {
    return RecordIterator<T>(map, offset);
  }

  /** Standard iterator end(). **/
  RecordIterator<T> end() const {
    return RecordIterator<T>(map, size);
  }

protected:
  // The upstream mapped records object
  const MappedRecords<T> &map;

  // The offset within the mapped file
  const size_t offset;

  // Mapped size
  const size_t size;
};

/**
* Records iterator; used by MappedRecords<T> to iterate records.
**/
template <typename T>
class RecordIterator: public std::iterator<std::input_iterator_tag, T> {
public:
  RecordIterator(const MappedRecords<T> &map, size_t offs): map(map), current(offs), offset(offs), size(map.get_size()) {
    assert(offset <= size);
    if (offset != size) {
      // Tune for linear read
      map.read_tune(offset, false);

      // Read first record
      map.get_record(record, offset);
    }
  }

  /** Standard iterator operator++. **/
  RecordIterator& operator++() {
    current = offset;
    map.get_record(record, offset);
    return *this;
  }

  /** Standard iterator operator!=. **/
  bool operator != (const RecordIterator<T> &other) const {
    return &map != &other.map
      || current != other.current;
  }

  /** Standard iterator operator*. **/
  const T& operator * () const {
    return record;
  }

protected:
  // The upstream mapped records object
  const MappedRecords<T> &map;

  // Current offset
  size_t current;

  // Next offset
  size_t offset;

  // Mapped size
  const size_t size;

  // Temporary object placeholder
  T record;

private:
  /* Forbidden foes */
  RecordIterator() = delete;
  RecordIterator& operator=(const RecordIterator&) = delete;
};

#endif
