bitbuf
======

**bitbuf** is a library for convenient and efficient handling of dynamic bit
arrays.  It is a simple data structure that stores a set of bits fully packed
in an array of bytes.  It aims to provide a memory-efficient implementation of
a variety of common arithmetic and logical operations on bit sets, such as
dynamic concatenation, bitwise operations, logical shift, numerical conversion
and pattern matching.

**Currently undergoing a rewrite in C++.**  For the original C implementation,
please contact me by mail.

## Usage

```c++
#include "bitbuf.h"

int main() {
  Bitbuf b {"0x48656c6c6f2c20"};

  b.append(Bitbuf {"0x626974627566"});
  b.append_byte(0x2e);

  std::cout << b.dump() << std::endl;
  return 0;
}
```

bitbuf is a source-only library.  Pull [bitbuf.cpp](bitbuf.cpp) and
[bitbuf.h](bitbuf.h) directly to your project and include into the build.
Requires compiler support for C++11.

For a full list of features and documentations on each functions, refer to [the
header file](bitbuf.h).

You can run unit tests by ``make test``.

## Design quirks

The main goal of this library is to implement common bitset operations in a
memory-efficient way.  As an example, the pattern matching function ``find()``
requires zero heap memory allocation for arbitrary patterns and match count.

The library is also written in a way to boost the possibility of vector
optimizations.  Most of the bulk operations such as append, bitshift, logical
OR, AND, XOR, produces SSE/AVX instructions in the binary when compiled with
-O3 -march=native. (tested with GCC 4.4.7 and Clang 3.4.) Specifically,
``find()`` is written using an compiler intrinsic that produces SSE4 popcount
for efficient hamming weight calculation.

## Comparisons to existing implementations

* boost::dynamic_bitset (C++)
  * **Pros**: Mature, time-tested implementation of a space-efficient dynamic
    bit array. Closely follows C++ container specifications such as iterators
    and element access by reference.
  * **Cons**: Lacks bulk bit operations such as sub, reverse and invert.  No
    support for construction with format string, numerical conversion or
    pattern matching.
* std::vector<bool> (C++)
  * **Pros**: STL implementation of a dynamic bit array.
  * **Cons**: Lacks common bit operations such as XOR and bitshift.
* std::bitset (C++)
  * **Pros**: STL implementation of a versatile fixed-size bit sequence.
  * **Cons**: No support for dynamic array resizing.
