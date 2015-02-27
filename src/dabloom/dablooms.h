/*
 *用途：网络重复数据删除支持可扩展的计数型Bloom Filter实现
 *功能描述：建立高效的指纹内存索引，确定数据块(chunk)是否重复，该实现主要分为三大模块：
 *      1. 数据位图操作；
 *      2. 计数型Bloom Filter实现
 *      3. 可扩展的计数型Bllom Filter实现
 *说明：本实现参考Justin Hines的dabloom的开源实现
 *    （http://word.bitly.com/post/28558800777/dablooms-an-open-source-scalable-counting-bloom）
 *     为华为公司(http://www.huawei.com/)的数据重复删除项目使用
 *创建日期：2013.2.2
 *修改日期：2013.2.7 : 修改关键字索引类型，支持murmurhash 64位为关键字（Murmurhash 分片后已计算）
 	 	  2013.2.8 : 参考 Scalable Bloom Filters
 	 	  http://www.sciencedirect.com/science/article/pii/S0020019006003127
 	 	  增加BF大小控制参数Scalable growth=2, 增加错误率控制参数r=0.8
 	 	  修改Counting Bloom Filter（CBF）初始化参数实现，修改Scalable CBF动态扩展策略和相关初始化

 *修改作者：刘屹
 */

#ifndef __BLOOM_H__
#define __BLOOM_H__


#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
enum{
	SCALABLE_GROWTH_RATIO=2
};
#define ERROR_TIGHTENING_RATIO 0.8

const char *dablooms_version(void);

typedef struct {
    size_t bytes;
    int    fd;
    unsigned char  *array;
} bitmap_t;


bitmap_t *bitmap_resize(bitmap_t *bitmap, size_t old_size, size_t new_size);
bitmap_t *new_bitmap(int fd, size_t bytes);

int bitmap_increment(bitmap_t *bitmap, unsigned int index, long offset);
int bitmap_decrement(bitmap_t *bitmap, unsigned int index, long offset);
int bitmap_check(bitmap_t *bitmap, unsigned int index, long offset);
int bitmap_flush(bitmap_t *bitmap);

void free_bitmap(bitmap_t *bitmap);

typedef struct {
    uint64_t id;
    uint32_t count;
    uint32_t _pad;
} counting_bloom_header_t;


typedef struct {
    counting_bloom_header_t *header;
    unsigned int capacity;
    long offset;
    unsigned int counts_per_func;
    uint32_t *hashes;
    size_t nfuncs;
    size_t size;
    size_t num_bytes;
    double error_rate;
    bitmap_t *bitmap;
} counting_bloom_t;

int free_counting_bloom(counting_bloom_t *bloom);
counting_bloom_t *new_counting_bloom(size_t bloom_size, double error_rate, const char *filename);
counting_bloom_t *new_counting_bloom_from_file(size_t bloom_size, double error_rate, const char *filename);
int counting_bloom_add(counting_bloom_t *bloom, KEY_TYPE s);
int counting_bloom_remove(counting_bloom_t *bloom, KEY_TYPE s);
int counting_bloom_check(counting_bloom_t *bloom,  KEY_TYPE s);

typedef struct {
    uint64_t max_id;
    uint64_t mem_seqnum;
    uint64_t disk_seqnum;
} scaling_bloom_header_t;

typedef struct {
    scaling_bloom_header_t *header;
    size_t scaling_bloom_size;
    unsigned int num_blooms;
    size_t num_bytes;
    double error_rate;
    int fd;
    counting_bloom_t **blooms;
    bitmap_t *bitmap;
} scaling_bloom_t;

scaling_bloom_t *new_scaling_bloom(size_t bloom_size, double error_rate, const char *filename);
scaling_bloom_t *new_scaling_bloom_from_file(size_t bloom_size, double error_rate, const char *filename);
int free_scaling_bloom(scaling_bloom_t *bloom);
int scaling_bloom_add(scaling_bloom_t *bloom, KEY_TYPE s, uint64_t id);
int scaling_bloom_remove(scaling_bloom_t *bloom,KEY_TYPE s, uint64_t id);
int scaling_bloom_check(scaling_bloom_t *bloom, KEY_TYPE s);
int scaling_bloom_flush(scaling_bloom_t *bloom);
uint64_t scaling_bloom_mem_seqnum(scaling_bloom_t *bloom);
uint64_t scaling_bloom_disk_seqnum(scaling_bloom_t *bloom);

#ifdef __cplusplus
}
#endif

#endif
