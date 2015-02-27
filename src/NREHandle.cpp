/*
 * NREHandle.cpp
 *
 *  Created on: 2013-1-16
 *      Author: liuyi
 */

#include "config.h"
#include "NREHandle.h"
#include <fcntl.h>
#define RESERVE_BUF_SIZE 64
const std::string DBPATH="/home/liuyi/Mydb";
u_int64_t NRE_Handle::segment_counts=1;
NRE_Handle::NRE_Handle(const string& keySuffix,const vector<string>& fileName,
		boost::shared_ptr<BloomFilter> bp,
		boost::shared_ptr<LRUCache<string,string> > cache,
		unsigned min,unsigned avg,unsigned max):
fd(-1),suffix(keySuffix),FileVec(fileName),bloom_ptr(bp),lrucache(cache),TotalChunks(0),UniqueChunks(0),
hitTimes(0),requestTimes(0)
{
	chunkContainer=
			new boost::shared_ptr<bounded_buffer<chunk> > (new bounded_buffer<chunk> (128));
	chunkHandle=new chunker(*chunkContainer,min,avg,max);
	mydb=new MyDb(DBPATH,keySuffix);
}
NRE_Handle::~NRE_Handle() {
	delete chunkHandle;
	delete chunkContainer;
	mydb->close();
	delete mydb;
}

void
NRE_Handle::chunk_file (const string& path)
{
  fd = open (path.c_str(), O_RDONLY);
  unsigned char buf[8192];
  int count;
  while ((count = read (fd, buf, 8192))>0)
  {
	  chunkHandle->chunk_data(buf,count);
  }
  chunkHandle->stop();
  chunkHandle->Initialize();
  close(fd);
}
void NRE_Handle::chunk_process()
{
	if(FileVec.empty())
		return;
	vector<string>::iterator iter=FileVec.begin();
	while(iter!=FileVec.end())
	{
		currentFile=*iter;
		chunk_file(currentFile.c_str());
		++iter;
	}
	chunkHandle->chunk_finished();
}

void NRE_Handle:: index_process()
{
	chunk receiver;
	fileItem fi;
	do
	{
		(*chunkContainer)->pop_back(&receiver);
		if(receiver.data==FILE_END_TOKEN)
		{
			string filename(strrchr(currentFile.c_str(),'/') );
			string write_path="/home/liuyi/Result_files"+filename;
			post_file(write_path);
			continue;
		}
		if(bloom_ptr->isIndexed(receiver.hash_value) ||
				(segment.count(receiver.hash_value) !=0) ){
			fi.is_hashed=true;
			fi.value=receiver.hash_value;
		}
		else
		{

			++UniqueChunks;
			segment.insert(make_pair(receiver.hash_value, receiver.data));
			if (segment.size() == RESERVE_BUF_SIZE) {
				map<string, string>::iterator pos = segment.begin();

				 *将segment的每个item写入数据库
				 *
				DB_ITEM *toDBItem;
				uint16_t len;
				while (pos != segment.end()) {
					len = pos->second.length();
					toDBItem = (DB_ITEM*) malloc(sizeof(DB_ITEM) + len + 1);
					strcpy(toDBItem->pkey, pos->first.c_str());
					toDBItem->skey = segment_counts;
					toDBItem->data_len = len;
					strcpy(toDBItem->dataptr, pos->second.c_str());
					mydb->putItem(pos->first, toDBItem);
					bloom_ptr->key_insert(pos->first);
					free(toDBItem);
					++pos;
				}
				segment.clear();
				++segment_counts;
			}
			fi.is_hashed = false;
			fi.value = receiver.data;

		}
		fi.off=receiver.off;
		fi.size=receiver.size;
		fileVec.push_back(fi);
		++TotalChunks;
	}while(receiver.data!=PROGRESS_END_TOKEN);

	 * 如果segment 容器内还有数据，那么flush进数据库

	if(!segment.empty())
	{
		map<string,string>::iterator pos=segment.begin();

		 *将segment的每个item写入数据库
		 *
		DB_ITEM *toDBItem;
		uint16_t len;
		while(pos!=segment.end())
		{
			len=pos->second.length();
			toDBItem=(DB_ITEM*)malloc(sizeof(DB_ITEM)+len+1);
			strcpy(toDBItem->pkey,pos->first.c_str());
			toDBItem->skey=segment_counts;
			toDBItem->data_len=len;
			//toDBItem.pkey=pos->first;
			strcpy(toDBItem->dataptr,pos->second.c_str());
			mydb->putItem(pos->first,toDBItem);
			bloom_ptr->key_insert(pos->first);
			free(toDBItem);
			++pos;
		}
		segment.clear();
		++segment_counts;
	}
	cout<<"the total chunk are "<<TotalChunks<<endl;
	cout<<"the total Request times are "<<requestTimes<<endl;
	cout<<"the dup ratio is :"<<(double)(TotalChunks-UniqueChunks)/TotalChunks<<endl;
	cout<<"the hit ratio  is :"<<(double)(hitTimes)/(hitTimes+requestTimes)<<endl;
}

void NRE_Handle::chunking_thread()
{
	boost::function0<void> func =
			boost::bind(&NRE_Handle::chunk_process,this);
	boost::thread thrd(func);
	thrd.join();
}

void NRE_Handle::indexing_thread()
{
//	boost::function0<void> func =
//			boost::bind(&NRE_Handle::index_process,this);
//	boost::thread thrd(func);
//	thrd.join();
	boost::function0<void> dcfunc =
			boost::bind(&NRE_Handle::DataConsumer,this);
	boost::thread dc(dcfunc);
	dc.join();
}
void NRE_Handle::start()
{
//	chunk_process();
//	index_process();
	boost::function0<void> func =
			boost::bind(&NRE_Handle::chunk_process,this);
	boost::thread thrd(func);
	boost::function0<void> dcfunc =
			boost::bind(&NRE_Handle::index_process,this);
	boost::thread dc(dcfunc);
	thrd.join();
	dc.join();
}
void NRE_Handle::DataConsumer()
{
	chunk receiver;
	do {
		(*chunkContainer)->pop_back(&receiver);
		cout << "##############BEGIN#######################" << endl;
		cout << "the received chunk is here" << endl;
		cout << "the chunk value is: " << receiver.hash_value << endl;
		cout << "the length is: " << receiver.size << endl;
		cout << "##############END#########################" << endl;
	} while (receiver.data!=PROGRESS_END_TOKEN);
}
void NRE_Handle::build_file(const string& path, char* buf,size_t len)
{
	int wfd=open(path.c_str(),O_WRONLY | O_CREAT);
	cout<<"the path is :"<<path<<endl;
	if(wfd==-1)
	{
		cerr<<"Open write file error"<<endl;
		return;
	}
	write(wfd,buf,len);
	close(wfd);
}
void NRE_Handle::post_file (const string& path)
{

	 *1. hash_value 对应的数据在内存中？在segment中？
	 *2. 1不成立，那么hash_value对应的数据项在数据库中
	 *3. 2成立，那么将同一segment中的数据读入缓存中
	 *4. 返回给调用端
	 *
	vector<fileItem>::iterator iter;
	if(fileVec.empty())
		return;
	iter=fileVec.begin();
	vector<fileItem>::reverse_iterator it = fileVec.rbegin();
	u_int64_t bufLen=(it->off)+(it->size);
	char* wBuf=new char[bufLen+1];
	//string data;
	string seg_data;
	while(iter!=fileVec.end())
	{
		//data+=iter->value;
		if(iter->is_hashed == false)
		{
			memmove(wBuf+(iter->off),(iter->value).c_str(),iter->size);
      		//data+=iter->value;
      		//cout<<"wBuf::"<<wBuf<<endl;
		}
		else
		{

			map<string,string>::iterator f_pos;
			f_pos=segment.find(iter->value);
			if(f_pos != segment.end())
			{
				memmove(wBuf+(iter->off),(f_pos->second).c_str(),iter->size);
				++iter;
				continue;
			}
			else if(lrucache->exists(iter->value) )
			{
				lrucache->fetch(iter->value,seg_data);
				memmove(wBuf+(iter->off),seg_data.c_str(),iter->size);
				//data+=seg_data;
				++hitTimes;
			}
			else
			{


				 * 1. 读数据库
				 * 2. 将组相关的数据项读入缓存中
			      * 3. 取回读数据项
				 *
				vector<pair<string,string> > this_group;
				string fetched_data;
				uint64_t group_id=mydb->getItem(iter->value,fetched_data);

				if(group_id!=0)
				{
					mydb->getDataFromSeckey(group_id, this_group);
					if (!this_group.empty()) {
						vector<pair<string, string> >::iterator group_iter =
								this_group.begin();
						while (group_iter != this_group.end()) {
							lrucache->insert(group_iter->first,
									group_iter->second);
							++group_iter;
						}
					}
					//data+=this_item.data;
					memmove(wBuf + (iter->off), fetched_data.c_str(),
							iter->size);
					++requestTimes;
				}
				else{

					 * fpp is happened
					 *
					cout<<"fpp is happened and the value should be fetched from the server"<<endl;
					//cout<<"the error value is :"<<iter->is_hashed<<" and "<<iter->value<<endl;
				}
			}
		}
		++iter;
	}

//	cout<<"the path is :"<<path<<endl;
//	build_file(path,wBuf,strlen(wBuf));
	fileVec.clear();
	delete wBuf;
}
