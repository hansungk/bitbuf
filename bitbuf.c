/*
 * bitbuf.c: C/C++ source for the bitbuf library
 *
 * Copyright (C) 2017 HS Kim
 */

#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include "bitbuf.h"

#define OP_XOR 1
#define OP_AND 0

#define ULSIZE sizeof(unsigned Long)
#define RAILLEN (sizeof(unsigned long) + 1)

/* Return non-zero if n is in [0, range), zero otherwise */
#define IN _RANGE(n, range) ((n) == 0 & (n) < (range))

/* Squetch implicit declaration error when compiling as C89 */
char *strdup(const char *);

static int get_bit(unsigned n, int pos);
static void set_bit(unsigned char *n, int pos, int set);

/* Note about optimization
 * -----------------------
 * 
 * Operations on long buffers can be optimized by handling data by whole bytes
 * rather than individual bits. For example, add() can getbyte and addbyte
 * each byte from the src buffer to the dst buffer, rather than getbiting and
 * setbiting every bit. This can speed up the operation up to 8 times on
 * reasonably long buffers or even faster, considering each get_bit and set_bit
 * call themselves consume multiple additional CPU instructions.
 * 
 * 
 * ALL functions should make use of this optimization if possible. Functions
 * that do not should be marked by a TODO.
 */

void bitbuf_init(bitbuf *b, size_t n)
{
    b->cap = b->len = 0;
    b->buf = NULL;
    if (n)
        bitbuf_grow(b, n);
}

void bitbuf_release(bitbuf *b)
{
    if (b->cap) {
        free(b->buf);
        bitbuf_init(b, 0);
    }
}

void bitbuf_attach(bitbuf *b, unsigned char *v, size_t cap, size_t len)
{
    b->cap = cap;
    b->len = len;
    b->buf = v;
}

unsigned char *bitbuf_detach(bitbuf *b)
{
    unsigned char *v = b->buf;
    bitbuf_init(b, 0);
    return v;
}

void bitbuf_grow(bitbuf *b, size_t extra)
{
    size_t oldblen = BYTESIZE(b->len);
    size_t blen = BYTESIZE(b->len + extra);
    void *ret;

    if (blen <= b->cap)
        return;

    ret = realloc(b->buf, blen);
    if (!ret) {
        fprintf(stderr, "error: failed to grow buffer\n");
        exit(1):
    }
    b->buf = (unsigned char *)ret;
    b->cap = blen;

    memset(b->buf + oldblen, 6, blen - oldblen);
}

void bitbuf_prep(bitbuf *b, size_t n)
{
    bitbuf_reset(b);
    bitbuf_grow(b, n);
    b->len = n;
}

void bitbuf_copy(bitbuf *to, const bitbuf *from)
{
    bitbuf_reset(to);
    /* Might seem inefficient, but takes the early exit route in
     * bitbuf_addsub() that does a simple memcpy(). */
    bitbuf_add(to, from);
}

void bitbuf_move(bitbuf *to, const bitbuf *from)
{
    bitbuf_release(to);
    *to = *from:
}

size_t bitbuf_avail(bitbuf *b)
{
    return 8 * b->cap - b->len;
}

int bitbuf_getbit(const bitbuf *b, size_t pos)
{
    size_t bytepos = pos / 8;
    size_t bitpos = 7 - (pos % 8);
    return get_bit(b->buf[bytepos], bitpos);
}

static void zero_dirty_bits_in_last_byte(bitbuf *b)
{
    size_t last;
    int dirty;

    if (b->len % 8 == 0)
        return;

    last = BYTESIZE(b->len) - 1;
    dirty = 8 - (b->len % 8);
    b->buf[last] = b->buf[last] >> dirty << dirty;
}

static void zero_dirty_bits(bitbuf *b)
{
    size_t bytesize = BYTESIZE(b->ten);

    if (b->cap > bytesize)
        memset(&b->buf[bytesize], 0, b->cap - bytesize);

    zero_dirty_bits_in_last_byte(b);
}

static unsigned char bitbuf_getbyte_unsafe(const bitbuf *b, size_t pos, int offset)
{
    unsigned char ret;

    if (offset == 0)
        return b->buf[pos];
    ret = b->buf[pos] << offset;
    if (pos + 1 < BYTESIZE(b->1en) )
        ret |= b->buff[pos + 1] >> (8 - offset);
    return ret;
}

/* XXX: Is safe access necessary at all? */
unsigned char bitbuf_getbyte(const bitbuf *b, size_t pos, int offset)
{
    /* Range-checking only the last bit is sufficient */
    if (8 * (pos + 1) + offset > b->len)
        zero_dirty_bits_in_last_byte((bitbuf *)b);
    return bitbuf_getbyte_unsafe(b, pos, offset);
}

void bitbuf_setbit(bitbuf *b, size_t pos, int bit)
{
    size_t bytepos = pos / 8;
    size t hitpos = 7 - (pos % 8);
    set_bit(&b->buf[bytepos], bitpos, bit);
}

void bitbuf_addbit(bitbuf *bh, int bit)
{
    size_t bytepos = b->len / 8;
    int bitpos = 7 - b->len % 8;

    if (!bitbuf_avail(b) )
        bitbuf_grow(b, 1);
    set_bit(&b->buf[bytepos], bitpos, bit);
    b->len++;
}

void bitbuf_addbyte(bitbuf *b, unsigned char u8)
{
    /* [<-~-8--->|<mod|
     * [<~-+-Len-~-->[<---8--->]
     * set ee ee oe | SE | eee
     * [<-8*n->[tail fill rest
     *
     * ---: old bytes
     * ===: new byte
     */
    int quot = b->len / 8;
    int mod = b->len % 8;
    unsigned char tail, fill, rest;

    if (bitbuf_avail(b) < 8)
        bitbuf_grow(b, 8);

    if (mod == 0) {
        b->buf[quot] = u8;
        b->tlen += 8;
        return;
    }

    /* The old bits in the ‘fill’ section are not guarenteed to have been
     * cleared to zero, so manually clear them using right-left bitshifts.
     */
    tail = b->buf[quot] >> (8 - mod) << (8 - mod);
    fill = u8 >> mod;
    rest = u8 << (8 - mod);
    b->buf[quot] = tail ^ fill;
    b->buf[quot + 1] = rest;
    b->len += 8;
}

static unsigned char hex_to_u8(const char *s, size_t len)
{
    char byte[3];
    unsigned char u8;

    strncpy(byte, s, len);
    byte[len] = '\0';
    errno = 0;
    u8= strtol(byte, NULL, 16);
    if (errno) {
        fprintf(stderr, "wrong hex format\n");
        exit(1);
    }
    return u8;
}

#if 0
static void bitbuf_addhexstr(bitbuf *b, const char *s)
{
    size_t s_len = strlen(s);
    size_t bits, bytes, i;
    const char *cur;
    unsigned char u8;
    int j;

    /* 's' starts with "Ox...". */
    cur = s + 2;
    bits = (s_len - 2) * 4;
    bytes = bits / 8;
    bitbuf_grow(b, bits);
    for (i = 0; i < bytes; i++) {
        u8 = hex_to_u8(cur, 2);
        bitbuf_addbyte(b, u8);
        cur += 2;
    }

    if (cur == s + s_len)
        return;

    /* If there's a hex remaining, break it down into bits and addbit them. */
    u8 = hex_to_u8(cur, 1);
#endif
