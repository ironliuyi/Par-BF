/*
 * BloomFilter.cpp
 *
 *  Created on: 2013-5-28
 *      Author: liuyi
 */

#include "BloomFilter.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <exception>
#include <math.h>

bool BloomFilter::isIndexed(const KEY_TYPE& key) {
	return (1 == scaling_bloom_check(sb, key)) ? true : false;
}

void BloomFilter::key_insert(const KEY_TYPE& key) {
	if (sync_count == sync_threshold) {
		bitmap_flush(sb->bitmap);
		sync_count = 0;
	}
	scaling_bloom_add(sb, key, id);
	++id;
	++sync_count;
}

void BloomFilter::key_delete(const KEY_TYPE& key) {
	scaling_bloom_remove(sb, key, id);
	--id;
}

void BloomFilter::sync_bf() const {
	bitmap_flush(sb->bitmap);
}
