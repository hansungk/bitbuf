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

TEST_CASE("append() subbuffer", "[append_sub]") {
  Bitbuf bs;
  bs.append_byte(0xF1);
  bs.append_byte(0xF2);
  bs.append_byte(0xF4);
  bs.append_byte(0xF8);

  SECTION("byte aligned") {
    {
      Bitbuf b;
      b.append(bs, 8, 24);
      REQUIRE(b.size() == 16);
      REQUIRE(b.data()[0] == 0xF2);
      REQUIRE(b.data()[1] == 0xF4);
    }
    {
      Bitbuf b;
      b.append(bs, 8, 15);
      REQUIRE(b.size() == 7);
      REQUIRE((b.data()[0] & 0xFE) == 0xF2);
    }
  }
  SECTION("not byte aligned") {
    {
      Bitbuf b;
      b.append(bs, 4, 18);
      REQUIRE(b.size() == 14);
      REQUIRE(b.data()[0] == 0x1F);
      REQUIRE((b.data()[1] & 0xFC) == 0x2C);
    }
    {
      Bitbuf b;
      b.append(bs, 15, 24);
      REQUIRE(b.size() == 9);
      REQUIRE(b.data()[0] == 0x7A);
      REQUIRE((b.data()[1] & 0x80) == 0x00);
    }
    {
      Bitbuf b;
      b.append(bs, 14, 32);
      REQUIRE(b.size() == 18);
      REQUIRE(b.data()[0] == 0xBD);
      REQUIRE(b.data()[1] == 0x3E);
      REQUIRE((b.data()[2] & 0xC0) == 0x00);
    }
  }
  SECTION("subbing to the last bit") {
    {
      Bitbuf b;
      b.append(bs, 1, 32);
      REQUIRE(b.size() == 31);
      REQUIRE(b.data()[0] == 0xE3);
      REQUIRE(b.data()[1] == 0xE5);
      REQUIRE(b.data()[2] == 0xE9);
      REQUIRE((b.data()[3] & 0xFC) == 0xF0);
    }
  }
  SECTION("adding 1-bit sub to a non-empty buffer") {
    {
      Bitbuf b;
      Bitbuf bs2;
      bs2.append_byte(0xDE);
      bs2.append_byte(0xAD);
      bs2.append_byte(0xBE);
      bs2.append_byte(0xEF);
      for (size_t i = 0; i < bs2.size(); i++) {
        b.append(bs2, i, i + 1);
      }
      REQUIRE(b.size() == bs2.size());
      REQUIRE(b.data()[0] == 0xDE);
      REQUIRE(b.data()[1] == 0xAD);
      REQUIRE(b.data()[2] == 0xBE);
      REQUIRE(b.data()[3] == 0xEF);
    }
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
