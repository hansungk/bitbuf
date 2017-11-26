#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "bitbuf.h"

TEST_CASE("Bitbuf initialization", "[init]") {
  {
    Bitbuf b;
    REQUIRE(b.size() == 0);
    REQUIRE(b.capacity() == 0);
  }

  {
    Bitbuf b{"01110"};
  }
}

TEST_CASE("push_back()", "[push_back]") {
  Bitbuf b;

  b.push_back(1);
  b.push_back(0);
  b.push_back(0);
  b.push_back(1);
  b.push_back(1);
  REQUIRE(b.data()[0] == 0x98);

  b.push_back(0);
  b.push_back(1);
  b.push_back(1);
  REQUIRE(b.data()[0] == 0x9b);

  // cross byte boundary
  b.push_back(1);
  REQUIRE(b.data()[0] == 0x9b);
  REQUIRE((b.data()[1] & 0x80)== 0x80);
}

TEST_CASE("append_byte()", "[append_byte]") {
  SECTION("byte aligned") {
    Bitbuf b;
    b.append_byte(0x78);
    REQUIRE(b.size() == 8);
    REQUIRE(b.data()[0] == 0x78);
  }

  SECTION("not byte aligned") {
    Bitbuf b;
    b.push_back(0);
    b.push_back(1);
    b.push_back(1);
    b.append_byte(0x78);
    REQUIRE(b.size() == 11);
    REQUIRE(b.data()[0] == 0x6F);
    REQUIRE((b.data()[1] & 0xE0) == 0x00);
  }
}

TEST_CASE("reserve()", "[reserve]") {
  Bitbuf b;

  b.reserve(0);
  REQUIRE(b.capacity() == 0);

  // block size == 8?
  // ceiling?
  b.reserve(16);
  REQUIRE(b.capacity() == 16);
  b.reserve(17);
  REQUIRE(b.capacity() == 24);
}
