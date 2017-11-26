/*
 * bitbuf.h: C/C++ header for the bitbuf library
 *
 * Copyright (C) 2017 HS Kim
 */

#ifndef _BITBUF_H_
#define _BITBUF_H_

#include <iostream>
#include <vector>

#if __cplusplus > 201402L
using std::byte;
#else
typedef unsigned char byte;
#endif

// A Bitbuf is a dynamic array that efficiently stores and handles bits in a
// byte-packed manner.  It supports a variety of operations such as dynamic
// concatenation, bitwise operations, logical shift, decimal conversion and
// pattern matching implemented in a memory-efficient way.
class Bitbuf {
public:
  // Make an empty buffer.
  Bitbuf() : Bitbuf(0) {}
  // Make a buffer with given number of zeros.
  Bitbuf(size_t n) : len(0), buf() {
    if (n)
      reserve(n);
  }
  // Make a buffer and fill it with bits that are parsed from the format
  // string.  This provides an easy way to make a buffer without manually
  // addbit()ing the bits.  The format is "0x...", "1010...", "0b1010..." or
  // any mix of the three, separated by a whitespace.
  Bitbuf(const std::string &s);
  // Make a buffer with the content of a subset of another buffer.  The
  // subbuffer range is [start, end).
  Bitbuf(const Bitbuf &b, size_t start, size_t end);
  // Explicit constructor.  Transfers the ownership of the memory, nullifying
  // `v`; this means `v` should not be used after the construction.
  //
  // Note that `len` should not exceed `v.size() * 8`.
  Bitbuf(size_t c, size_t l, std::vector<byte> &&v) : len(l), buf(std::move(v)) {}
  // Copy constructor. Make a new buffer that has the same content with b.
  Bitbuf(const Bitbuf &b) : Bitbuf() { /* this->copy_from(b); TODO */
  }
  // Move constructor.  Transfers the ownership of the buffer `b` to this.  It
  // effectively replaces the buffer content with that of `b`, and subsequently
  // releases the memory of `b`.
  Bitbuf(Bitbuf &&b) {
    // TODO
  }
  ~Bitbuf() {}

  // Return the bit length of this buffer.
  size_t size() const { return len; }
  // Return the number of bits that the buffer has currently allocated space
  // for.
  size_t capacity() const { return buf.capacity() * 8; }
  // Ensure that the amount of memory to store at least this number of bits
  // (note the unit) is available.  This is mostly used in inner functions, but
  // it can be used when you know a typical size for what you will add and want
  // to avoid repetitive reallocations.
  //
  // Note that this does not update the length of the bit array.  This means
  // consecutive calls to this function does not allocate new memory
  // repeatedly.
  void reserve(size_t n);
  // Return access pointer to the raw memory of the buffer.
  const byte *data() const { return buf.data(); }

  // Push a single bit at the end of this buffer.
  void push_back(bool bit);
  // Append another buffer at the end of this buffer.
  void append(const Bitbuf &b);
  // Append a subbuffer of range [start, end) of another buffer at the end of
  // this buffer.
  void append_sub(const Bitbuf &b, size_t start, size_t end);
  // Append given number of zeros at the end of this buffer.
  void append_zeros(size_t n);
  // Append a single 8-bit byte at the end of this buffer.
  void append_byte(byte u8);
  // Set a single bit at position n to the given bit.
  void set_bit(size_t n, bool bit);
  // Clear all the bits in the buffer to zero.
  void clear();

  // Reverse the order of bits of this buffer.
  void reverse() { reverse(0, len); }
  // Reverse the order bits in the range [start, end).
  void reverse(size_t start, size_t end);
  // Reverse every n bits in the buffer.
  void reverse_block(size_t n);
  // Invert the bits of the buffer, i.e., convert 0s to 1s and 1s to 0s.
  void invert() { invert(0, len); }
  // Invert the bits in the range [range, end), i.e., convert 0s to 1s and 1s
  // to 0s.
  void invert(size_t start, size_t end);

  /// Conversion
  // Convert the binary content of the buffer into a numerical value.
  unsigned long long numeric() const;

  // Dump the contents of the buffer to a string.  The format is compatible
  // with the input format used in Bitbuf(const string&).
  const std::string dump() const;

  /// Operators

  // Return the bit at position n in the buffer.
  bool operator[](size_t n) const;

  // Compare the unsigned numeric value of two buffers.
  bool operator>(const Bitbuf &b);
  bool operator==(const Bitbuf &b);
  bool operator<(const Bitbuf &b);

private:
  // The number of accessible bits in the buffer.
  // Must not exceed `buf.size() * 8`.
  size_t len;
  // Raw data buffer.
  std::vector<byte> buf;

  // Return the 8-bit byte value located at [8*pos + offset, 8*pos + offset + 8).
  byte byte_at_pos_offset(size_t pos, int offset) const;
};

// Minimum number of bytes that can pack `n` bits
constexpr size_t byte_count(size_t n) {
  return (n + 7) / 8;
}

// Byte size to read every cycle
constexpr int bufsize = 8192;

#ifndef __cplusplus
// Bitbuf C API.

// Initialize a buffer.  The second parameter is the number of bits to
// allocate, which can be zero or greater in case you want to prevent further
// reallocs.
//
// The reason this function initializes the buffer in-place, rather than
// returning a new one, is to enable using bitbuf as a stack variable.
void bitbuf_init(Bitbuf *, size_t);

// Initialize a buffer and fill it with bits that are parsed from the format
// string.  This provides an easy way to make a buffer without manually
// addbit()ing the bits.  The format is "0x...", "1010...", "0b1010..." or any
// mix of the three, separated by a whitespace.
void bitbuf_initstr(Bitbuf &, const char *);

#endif // __cplusplus
#endif // _BITBUF_H_
