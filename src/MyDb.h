/*
 *用途：网络重复数据删除key-value值对的数据库简单操作
 *功能描述：Berkeley DB简单实现数据插入、查找、删除操作，特别注意的是，数据库优化是需要特别定制的复杂过程，
 *        需进一步支持transaction相关操作，如rollback，backup以及数据恢复策略
 *说明：本实现为华为公司(http://www.huawei.com/)的数据重复删除项目使用
 *创建日期：2012.2.3
 *修改日期：2013.2.18 优化数据存储结构和效率
 *修改作者：刘屹
 */
#include <db_cxx.h>
#include <string.h>
#include <vector>
#include <set>
using namespace std;
/*
 * 支持变长数据的数据库存储单元
 */
#pragma pack(push)
#pragma pack(1)
typedef struct storeItem {
	KEY_TYPE pkey; // primary key
	uint64_t skey; //for second indexing
	uint16_t data_len; //字符长度
	char dataptr[1]; //指向数据字符首地址
	/*
	 * 下一步需要实现？
	 */
	// time_t visited_time;

	bool operator<(const storeItem& rhs) const
	{
		return (this->pkey < rhs.pkey);
	}
	bool operator==(const storeItem& rhs) const
	{
		return (this->pkey==rhs.pkey);
	}
}DB_ITEM;
#pragma pack(pop)

class MyDb
{
public:
// Constructor requires a path to the database,
// and a database name.
MyDb(const std::string &path, const std::string &dbName);
// Our destructor just calls our private close method.
~MyDb() {;};
inline Db &getDb() {return *db_;}
void putItem(const KEY_TYPE& key,DB_ITEM* value);
uint64_t getItem(const KEY_TYPE& key,string& value) const;
void deleteItem(const KEY_TYPE& key);
static void destroyEnv()
{
	if(dbenv!=NULL)
		delete dbenv;
	dbenv=NULL;
};
void getDataFromSeckey(uint64_t& ,vector<pair<KEY_TYPE,string> >&);
void deleteSegment(const KEY_TYPE& key);
void close();
uint64_t cleanObsoleteItem(const double freshTime=3600*24)
{
	return 0;
}
private:
Db *db_; //BDB 句柄
std::string dbFileName_; //数据库文件路径名
std::string secdbFileName_;//索引文件路径名
Db *secdb_; //索引句柄
u_int32_t cFlags_;

static DbEnv* dbenv; //BDB环境设置句柄
private:
// Make sure the default constructor is private
// We don't want it used.
// We put our database close activity here.
// This is called from our destructor. In
// a more complicated example, we might want
// to make this method public, but a private
// method is more appropriate for this example.
MyDb(const MyDb&);
MyDb& operator = (const MyDb&);
void buildSecIndex(); //构建基于段号(segment id)的索引

};



