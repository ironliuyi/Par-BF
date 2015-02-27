/*
 *用途：网络重复数据删除支持可扩展的计数型Bloom Filter接口，调用dabloom模块
 *功能描述：dablom的接口对象，支持指纹查找、插入、删除操作，统计目前的错误率（fpp）和 理论错误率,
          dablom的接口对象，支持指纹查找、插入、删除操作，统计目前的错误率（fpp）和 并统计所占内存大小

 *说明：本实现为华为公司(http://www.huawei.com/)的数据重复删除项目使用
 *创建日期：2013.3.9
 *修改作者：刘屹
 */

#ifndef BLOOMFILTER_H_
#define BLOOMFILTER_H_

#include "config.h"
#include "dabloom/dablooms.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <exception>
#include <math.h>

class BloomFilter
{
public:

	explicit BloomFilter(u_int64_t Capacity, double Error_rate,
			const char* Sb_file, bool isRebuild=false, uint32_t Sync_threshold=1024) :
			is_rebuild(isRebuild), capacity(Capacity), error_rate(Error_rate), sb_file(
					Sb_file), id(0), false_positive_num(0), true_positive_num(
					0), sync_count(0), sync_threshold(Sync_threshold)
	{
		if (is_rebuild) {
			sb = new_scaling_bloom_from_file(capacity, error_rate, sb_file);
		}
		else
			init();
	}

	~BloomFilter()
	{
		if(fp)
			fclose(fp);
		free_scaling_bloom(sb);
	};

	bool isIndexed(const KEY_TYPE& key);

    void key_insert(const KEY_TYPE& key);

    void key_delete(const KEY_TYPE& key);

	inline void inc_false_positvie_num()
	{
		++false_positive_num;
	};
	inline void inc_true_positive_num()
	{
		++true_positive_num;
	};
	inline double current_fpp() const
	{
		return (double)false_positive_num/(false_positive_num+true_positive_num);
	};
	inline double scaled_fpp() const
	{
		double P;
		for(unsigned i=0;i < sb->num_blooms;++i)
		{
			P *= 1-sb->error_rate * pow(ERROR_TIGHTENING_RATIO, i);
		}
		return (1-P);
	};
	inline void sync_bf() const;

	inline double get_BF_size() const
	{
		return ((double)sb->num_bytes/(0x1<<30));
	}
private:
	void init()
	{
	    if ((fp = fopen(sb_file, "r"))) {
	        fclose(fp);
	        remove(sb_file);
	    }
	    if (!(sb = new_scaling_bloom(capacity, error_rate, sb_file))) {
	    	DEBUG_PRINT("%s\n","ERROR: Could not create bloom filter");
	        throw "Bloom Filet Error";
	    }

	};
private:
    FILE *fp;
	scaling_bloom_t* sb;
	bool is_rebuild;
	uint64_t capacity;
	double error_rate;
	const char* sb_file;
	uint64_t id;
	uint64_t false_positive_num;
	uint64_t true_positive_num;
	uint32_t sync_count;
	uint32_t sync_threshold;
};

#endif /* BLOOMFILTER_H_ */
