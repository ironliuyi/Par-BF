/*
 * MurmurHash64B.h
 *
 *  Created on: 2013-5-10
//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment
// and endian-ness issues if used across multiple platforms.
 */

#ifndef MURMURHASH64_H_
#define MURMURHASH64_H_
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 64-bit hash for 64-bit platforms
static uint64_t MurmurHash64A(const void * key, uint16_t len, unsigned int seed);

// 64-bit hash for 32-bit platforms
static uint64_t MurmurHash64B(const void * key, uint16_t len, unsigned int seed);

//API
extern uint64_t MurmurHash64 (const void * key, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* MURMURHASH64B_H_ */
