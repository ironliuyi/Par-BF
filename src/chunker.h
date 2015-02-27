/*
 *用途：网络重复数据删除数据分片的实现
 *功能描述：利用CDC算法生成数据分片信息。信息保存在结构chunk中，包括hash指纹，数据chunk起始偏移量，长度以及chunk值
 *说明：本实现参考LBFS(LBFS: A Low-bandwidth Network File System，http://pdos.csail.mit.edu/lbfs/)策略，
 *     为华为公司(http://www.huawei.com/)的数据重复删除项目使用
 *创建日期：2012.10.22
 *修改日期：2013.1.15 支持murmurhash生成hash指纹
 *修改作者：刘屹
 */

#ifndef CHUNKER_H_
#define CHUNKER_H_
#include "config.h"
#include "rabinpoly.h"
#include "bounded_buffer.h"
#include <string.h>
using namespace std;

#ifdef MURMURHASH
typedef uint64_t HASH_LEN ;
#else
typedef uint128_t HASHLEN;
#endif

extern const string FILE_END_TOKEN;
extern const string PROGRESS_END_TOKEN;

//分片三元组<min_chunk,avg_chunk,max_chunk>初值
const unsigned MIN_CHUNK_SIZE=96;
const unsigned AVG_CHUNK_SIZE=128;
const unsigned MAX_CHUNK_SIZE=256;

//分片元信息结构
struct chunk
{
	HASH_LEN hash_value; //Hash指纹
	uint64_t off;  //切片起始偏移值
	size_t size; 	//切片大小
	string data;	//切片数据

	chunk(HASH_LEN hv, int o, size_t s, const string& d) :
		hash_value(hv),off(o), size(s), data(d) {
	};

	chunk() :hash_value(0), off(0), size(0), data("0") {
	};

	const chunk& operator=(const chunk &rhs){
		hash_value = rhs.hash_value;
		off = rhs.off;
		size = rhs.size;
		data = rhs.data;
		return *this;
	};

	chunk(const chunk &rhs) {
		hash_value = rhs.hash_value;
		off = rhs.off;
		size = rhs.size;
		data = rhs.data;
	};
};

//CDC算法调用Rabin Hash将数据文件分片。
class chunker{
private:

	//分片三元组<min_chunk,avg_chunk,max_chunk>具体值
	unsigned min_chunk_size;
	unsigned avg_chunk_size;
	unsigned max_chunk_size;

	//统计大于max_chunk_size和小于min_chunk_size的定长切片数;
	unsigned min_size_suppress;
	unsigned max_size_suppress;

	//CDC算法，字符掩码值，例如平均切片大小为4KB(2^12),那么字符掩码长度为log2(4096)
    unsigned int chunkmask;

    //Rabin Hash 滑动窗口对象
	window _w;

	//偏移量位置
	size_t _last_pos;
	size_t _cur_pos;

    //buffer队列指针，chunker用来生产chunk，从而索引线程能确定重复数据片
	boost::shared_ptr<bounded_buffer<chunk> > buffer;

	//内部buffer，基于滑动窗口保存文本信息
	unsigned char* _hbuf;
	unsigned int _hbuf_size;
	unsigned int _hbuf_cursor;

	//保存数据字符串：data至hbuf
	void handle_hash(const unsigned char *data, size_t size);

public:
	chunker(boost::shared_ptr<bounded_buffer<chunk> > sp,
			unsigned mincs=MIN_CHUNK_SIZE,
			unsigned avgcs=AVG_CHUNK_SIZE,
			unsigned maxcs=MAX_CHUNK_SIZE);
	~chunker();

	void Initialize();
	size_t cur_pos() const {return _cur_pos;}

	//文本分片信息尾部处理，发送文本处理结束标识：FILE_END_TOKEN
	void stop();

	//CDC算法实现函数
	void chunk_data(const unsigned char *data,size_t size);

    //分片Debug使用
	void showData();

	//分片线程结束，发送线程处理结束标识：PROGRESS_END_TOKEN
	void chunk_finished();
};

#endif /* CHUNKER_H_ */
