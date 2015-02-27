/*
 * NREHandle.h
 *
 *  Created on: 2013-1-16
 *      Author: liuyi
 */

#ifndef NREHANDLE_H_
#define NREHANDLE_H_
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "chunker.h"
#include "BloomFilter.h"
#include "lru_cache.h"
#include "MyDb.h"

using namespace std;
const unsigned int BUFLEN=4096;
class NRE_Handle {
public:
	NRE_Handle(const string& keySuffix,const vector<string>& fileName,
			boost::shared_ptr<BloomFilter> bp,
			boost::shared_ptr<LRUCache<string,string> > cache,
			unsigned min,unsigned avg,unsigned max);
	virtual ~NRE_Handle();
public:
	void start();
	void DataConsumer();
	void chunk_file (const string& path);
	void chunk_process();
	void chunking_thread();
private:
	void post_file  (const string& path);
	void build_file (const string& path,char* buf,size_t len);
	//void chunk_process();
	void index_process();
	void database_process();
	//void chunking_thread();
	void indexing_thread();
private:
    int fd;
    string suffix;
    vector<string> FileVec;
    string currentFile;
    chunker *chunkHandle;
    boost::shared_ptr<BloomFilter> bloom_ptr;
    boost::shared_ptr<bounded_buffer<chunk> > *chunkContainer;
    boost::shared_ptr<LRUCache<string,string> > lrucache;
    unsigned char file_buf[BUFLEN];
    map<KEY_TYPE,string> segment;
    static u_int64_t segment_counts;
    struct fileItem
    {
    	bool is_hashed;
		unsigned long long  off;
		size_t size;
		string value;
    };
    vector<fileItem> fileVec;

    /*DB Handle*/
    MyDb *mydb;

    // 重复数据率统计量
    uint64_t TotalChunks;
    uint64_t UniqueChunks;
    uint64_t hitTimes;
    uint64_t requestTimes;
};

#endif /* NREHANDLE_H_ */
