#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define TEST_INIT(str, fcn_name) \
	static void fcn_name##_test(void) { \
		unsigned int x, total1 = 0, total2 = 0; \
		struct timeval start, end; \
		time_t elapsed_time_us, unrolled_elapsed_time_us; \
		\
		gettimeofday(&start, NULL); \
		for(x = 0; x < 64*1024*1024; x++) { \
			total1 += fcn_name(x); \
		} \
		for(x = UINT_MAX; x > UINT_MAX - 64*1024*1024; x--) { \
			total1 += fcn_name(x); \
		} \
		gettimeofday(&end, NULL); \
		\
		elapsed_time_us = end.tv_usec - start.tv_usec; \
		elapsed_time_us += 1000000l * (end.tv_sec - start.tv_sec); \
		\
		gettimeofday(&start, NULL); \
		for(x = 0; x < 64*1024*1024; x+=4) { \
			total2 += fcn_name(x); \
			total2 += fcn_name(x+1); \
			total2 += fcn_name(x+2); \
			total2 += fcn_name(x+3); \
		} \
		for(x = UINT_MAX; x > UINT_MAX - 64*1024*1024; x-=4) { \
			total2 += fcn_name(x); \
			total2 += fcn_name(x-1); \
			total2 += fcn_name(x-2); \
			total2 += fcn_name(x-3); \
		} \
		gettimeofday(&end, NULL); \
		unrolled_elapsed_time_us = end.tv_usec - start.tv_usec; \
		unrolled_elapsed_time_us += 1000000l * (end.tv_sec - start.tv_sec); \
		\
		printf("%s %6.3fs (%.3fs unrolled) %d %d\n", str, elapsed_time_us / 1e6, unrolled_elapsed_time_us / 1e6, total1, total2); \
	}

static const unsigned char bit_lookup[256] = {
	0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,1,2,2,3,2,
	3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,1,2,2,3,2,3,3,4,2,3,
	3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,
	6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,
	3,4,4,5,3,4,4,5,4,5,5,6,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,
	5,5,6,5,6,6,7,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,
	6,7,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

inline static unsigned int simple(unsigned int x) {
	unsigned int c;
	for(c = 0; x; x >>= 1) {
		c += x & 1;
	}
	return c;
}

inline static unsigned int lookup1(unsigned int x) {
	return bit_lookup[x & 0xFF] +
	       bit_lookup[(x >> 8) & 0xFF] +
	       bit_lookup[(x >> 16) & 0xFF] +
	       bit_lookup[x >> 24];
}

inline static unsigned int lookup2(unsigned int x) {
	unsigned char *y = (unsigned char *)&x;
	return bit_lookup[y[0]] +
		   bit_lookup[y[1]] +
		   bit_lookup[y[2]] +
		   bit_lookup[y[3]];
}

inline static unsigned int kernighan(unsigned int x) {
	unsigned int c;
	for(c = 0; x; c++) {
		x &= x - 1;
	}
	return c;
}

inline static unsigned int mod(unsigned int x) {
	unsigned int c;
	c = ((x & 0xFFF) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1F;
	c += (((x & 0xFFF000) >> 12) * 0x1001001001001ULL&0x84210842108421ULL) % 0x1F;
	c += ((x >> 24) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1F;
	return c;
}

inline static unsigned int parallel(unsigned int x) {
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	return (((x + (x >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

inline static unsigned int builtin(unsigned int x) {
	/* compiles down to popcntl if available */
	return __builtin_popcount(x);
}

TEST_INIT("simple    ", simple)
TEST_INIT("lookup1   ", lookup1)
TEST_INIT("lookup2   ", lookup2)
TEST_INIT("kernighan ", kernighan)
TEST_INIT("mod       ", mod)
TEST_INIT("parallel  ", parallel)
TEST_INIT("builtin   ", builtin)

int main() {
	simple_test();
	lookup1_test();
	lookup2_test();
	kernighan_test();
	mod_test();
	parallel_test();
	builtin_test();
	return EXIT_SUCCESS;
}

