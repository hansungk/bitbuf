#include "bitbuf.h"
#include <sstream>

namespace {

void set_bit(byte &n, int pos, int set) {
  unsigned mask = 0x01 << pos;
  if (set)
    n |= mask;
  else
    n &= ~mask;
}

void append_binary_str(Bitbuf &b, const std::string &s) {
  auto start = s.cbegin();
  if (s.compare(0, 2, "0b") == 0)
    start += 2;
  for (auto it = start; it != s.cend(); it++) {
    b.push_back(*it - '0');
  }
}

}

Bitbuf::Bitbuf(const std::string &s) : Bitbuf() {
  std::string word;
  for (std::istringstream iss(s); iss >> word; ) {
    append_binary_str(*this, word);
  }
}

void Bitbuf::reserve(size_t n) {
  buf.reserve(byte_count(n));
}

void Bitbuf::push_back(bool bit) {
  size_t byte_pos = len / 8;
  int bit_pos = 7 - len % 8;

  if (len == capacity())
    buf.push_back(0);
  ::set_bit(buf[byte_pos], bit_pos, bit);
  len++;
}

void Bitbuf::append_byte(byte u8) {
  //         |<---8--->|<mod|
  // |<----len--->|<---8--->|
  // |-------|----+====|====+XXXX|
  // |<-8*n->|tail fill rest
  //
  // ---: old bits
  // ===: added bits
  // XXX: unused bits
  int quot = len / 8;
  int mod = len % 8;
  byte tail, fill, rest;

#if 0 // No need to micromanage growth anymore.
  if (capacity() - len < 8)
    reserve(len + 8);
#endif

  if (mod == 0) {
    buf.push_back(u8);
    len += 8;
    return;
  }

  // The old bits in the 'fill' section are not guaranteed to be cleared to
  // zero, so manually clear them using two consequtive bitshifts.
  tail = buf[quot] >> (8 - mod) << (8 - mod);
  fill = u8 >> mod;
  rest = u8 << (8 - mod);
  buf[quot] = tail ^ fill;
  buf.push_back(rest);
  len += 8;
}

void Bitbuf::append_sub(const Bitbuf &ba, size_t start, size_t end) {
  // Efficient append when byte aligned
  if (len % 8 == 0 && start % 8 == 0) {
    buf.insert(buf.end(), ba.buf.begin(), ba.buf.end());
    return;
  }

  size_t gap = end - start;
  reserve(len + gap);

  size_t old_len = len;
  for (size_t i = 0; i < byte_count(gap); i++) { // XXX might exceed reserved space
    byte byte_ = ba.byte_at_pos_offset(start / 8 + i, start % 8);
    append_byte(byte_);
  }
  len = old_len + gap;
}

byte Bitbuf::byte_at_pos_offset(size_t pos, int offset) const {
  if (offset == 0)
    return buf[pos];

  byte ret = buf[pos] << offset;
  if (pos + 1 < byte_count(len))
    ret |= (buf[pos + 1] >> (8 - offset));
  return ret;
}
