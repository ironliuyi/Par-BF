/*
 * rabinpoly.h
 *
 *  Created on: Oct 22, 2012
 *      Author: ubuntu
 */

#ifndef RABINPOLY_H_
#define RABINPOLY_H

#include <sys/types.h>
#include <string.h>

u_int64_t polymod (u_int64_t nh, u_int64_t nl, u_int64_t d);
u_int64_t polygcd (u_int64_t x, u_int64_t y);
void polymult (u_int64_t *php, u_int64_t *plp, u_int64_t x, u_int64_t y);
u_int64_t polymmult (u_int64_t x, u_int64_t y, u_int64_t d);
bool polyirreducible (u_int64_t f);
//u_int64_t polygen (u_int degree);

const int DEFAULT_SLIDE_WINDOW_SIZE=48;
class rabinpoly {
  int shift;
  u_int64_t T[256];		// Lookup table for mod
  void calcT ();
public:
  const u_int64_t poly;		// Actual polynomial

  explicit rabinpoly (u_int64_t poly);
  u_int64_t append8 (u_int64_t p, u_char m) const
    { return ((p << 8) | m) ^ T[p >> shift]; }
};

class window : public rabinpoly {
public:
  enum {size = DEFAULT_SLIDE_WINDOW_SIZE};
  //enum {size = 24};
private:
  u_int64_t fingerprint;
  int bufpos;
  u_int64_t U[256];
  u_char buf[size];

public:
  window (u_int64_t poly);
  u_int64_t slide8 (u_char m) {
    if (++bufpos >= size)
      bufpos = 0;
    u_char om = buf[bufpos];
    buf[bufpos] = m;
    return fingerprint = append8 (fingerprint ^ U[om], m);
  }
  void reset () {
    fingerprint = 0;
    bzero (buf, sizeof (buf));
  }
};


#endif /* RABINPOLY_H_ */
