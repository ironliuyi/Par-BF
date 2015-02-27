/*
 * msb.h
 *
 *  Created on: Oct 22, 2012
 *      Author: ubuntu
 */

#ifndef MSB_H_
#define MSB_H_
#include <sys/types.h>

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else /* !__cplusplus */
#define EXTERN_C extern
#endif /* !__cplusplus */

/*
 * Routines for calculating the most significant bit of an integer.
 */

EXTERN_C const char bytemsb[];

/* Find last set (most significant bit) */
static inline u_int fls32 (u_int32_t v)
{
  if (v & 0xffff0000) {
    if (v & 0xff000000)
      return 24 + bytemsb[v>>24];
    else
      return 16 + bytemsb[v>>16];
  }
  if (v & 0x0000ff00)
    return 8 + bytemsb[v>>8];
  else
    return bytemsb[v];
}

/* Ceiling of log base 2 */
//static inline int
//  log2c32 (u_int32_t v)
//  {
//    return v ? (int) fls32 (v - 1) : -1;
//  }

//static inline u_int fls64 (u_int64_t) __attribute__ ((const));
static inline char fls64 (u_int64_t v)
{
  u_int32_t h;
  if ((h = v >> 32))
    return 32 + fls32 (h);
  else
    return fls32 ((u_int32_t) v);
}

static inline int log2c64 (u_int64_t) __attribute__ ((const));
static inline int
log2c64 (u_int64_t v)
{
  return v ? (int) fls64 (v - 1) : -1;
}

#define fls(v) (sizeof (v) > 4 ? fls64 (v) : fls32 (v))
#define log2c(v) (sizeof (v) > 4 ? log2c64 (v) : log2c32 (v))

/*
 * For symmetry, a 64-bit find first set, "ffs," that finds the least
 * significant 1 bit in a word.
 */

EXTERN_C const char bytelsb[];

static inline u_int
ffs32 (u_int32_t v)
{
  if (v & 0xffff) {
    if (int vv = v & 0xff)
      return bytelsb[vv];
    else
      return 8 + bytelsb[v >> 8 & 0xff];
  }
  else if (int vv = v & 0xff0000)
    return 16 + bytelsb[vv >> 16];
  else if (v)
    return 24 + bytelsb[v >> 24 & 0xff];
  else
    return 0;
}

static inline u_int
ffs64 (u_int64_t v)
{
  u_int32_t l;
  if ((l = v & 0xffffffff))
    return fls32 (l);
  else if ((l = v >> 32))
    return 32 + fls32 (l);
  else
    return 0;
}

#define ffs(v) (sizeof (v) > 4 ? ffs64 (v) : ffs32 (v))

#undef EXTERN_C

#endif /* MSB_H_ */
