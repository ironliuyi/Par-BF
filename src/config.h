#include<stdint.h>

#define MURMURHASH 1
#define DEBUG 1
//typedef unsigned long long uint64_t;
#if ( defined(__LP64__) || defined(_LP64) )
#define BUILD_64 1
#else
#define BUILD_64 0
#endif

#ifdef DEBUG
# define DEBUG_PRINT( fmt, ... ) \
	fprintf(stderr, "DEBUG: %s:%u - " fmt,  __FILE__, __LINE__, __VA_ARGS__)
#else
# define DEBUG_PRINT(fmt, ...) do {} while (0)
#endif

#ifdef MURMURHASH
typedef uint64_t KEY_TYPE;
#else
typedef char* KEY_TYPE;
#endif


