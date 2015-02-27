/*
 * rabinpoly.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: ubuntu
 */

#include "rabinpoly.h"
#include "msb.h"
#include <assert.h>
#define INT64(n) n##LL
#define MSB64 INT64(0x8000000000000000)


u_int64_t
polymod (u_int64_t nh, u_int64_t nl, u_int64_t d)
{
  assert (d);
  int k = fls64 (d) - 1;
  d <<= 63 - k;

  if (nh) {
    if (nh & MSB64)
      nh ^= d;
    for (int i = 62; i >= 0; i--)
      if (nh & INT64 (1) << i) {
	nh ^= d >> (63 - i);
	nl ^= d << (i + 1);
      }
  }
  for (int i = 63; i >= k; i--)
    if (nl & INT64 (1) << i)
      nl ^= d >>( 63 - i);
  return nl;
}

u_int64_t
polygcd (u_int64_t x, u_int64_t y)
{
  for (;;) {
    if (!y)
      return x;
    x = polymod (0, x, y);
    if (!x)
      return y;
    y = polymod (0, y, x);
  }
  return 0;
}

void
polymult (u_int64_t *php, u_int64_t *plp, u_int64_t x, u_int64_t y)
{
  u_int64_t ph = 0, pl = 0;
  if (x & 1)
    pl = y;
  for (int i = 1; i < 64; i++)
    if (x & (INT64 (1) << i)) {
      ph ^= y >> (64 - i);
      pl ^= y << i;
    }
  if (php)
    *php = ph;
  if (plp)
    *plp = pl;
}

u_int64_t
polymmult (u_int64_t x, u_int64_t y, u_int64_t d)
{
  u_int64_t h, l;
  polymult (&h, &l, x, y);
  return polymod (h, l, d);
}

bool
polyirreducible (u_int64_t f)
{
  u_int64_t u = 2;
  int m = (fls64 (f) - 1) >> 1;
  for (int i = 0; i < m; i++) {
    u = polymmult (u, u, f);
    if (polygcd (f, u ^ 2) != 1)
      return false;
  }
  return true;
}

/*u_int64_t
polygen (u_int degree)
{
  assert (degree > 0 && degree < 64);
  u_int64_t msb = INT64 (1) << degree;
  u_int64_t mask = msb - 1;
  u_int64_t f;
  int rfd = open (SFS_DEV_RANDOM, O_RDONLY);
  if (rfd < 0)
    printf("%s: %m\n", SFS_DEV_RANDOM);
  do {
    if (read (rfd, &f, sizeof (f)) != implicit_cast<ssize_t> (sizeof (f)))
      fatal ("%s: read failed\n", SFS_DEV_RANDOM);
    f = (f & mask) | msb;
  } while (!polyirreducible (f));
  close (rfd);
  return f;
}*/

void
rabinpoly::calcT ()
{
  assert (poly >= 0x100);
  int xshift = fls64 (poly) - 1;
  shift = xshift - 8;
  u_int64_t T1 = polymod (0, INT64 (1) << xshift, poly);
  for (int j = 0; j < 256; j++)
    T[j] = polymmult (j, T1, poly) | ((u_int64_t) j << xshift);
}

rabinpoly::rabinpoly (u_int64_t p)
  : poly (p)
{
  calcT ();
}

window::window (u_int64_t poly)
  : rabinpoly (poly), fingerprint (0), bufpos (-1)
{
  u_int64_t sizeshift = 1;
  for (int i = 1; i < size; i++)
    sizeshift = append8 (sizeshift, 0);
  for (int i = 0; i < 256; i++)
    U[i] = polymmult (i, sizeshift, poly);
  bzero (buf, sizeof (buf));
}
