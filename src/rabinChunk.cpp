/*
 ============================================================================
 Name        : RabinPrint.cpp
 Author      : liuyi
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C++,
 ============================================================================
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include <boost/thread.hpp>
#include "chunker.h"
#include "FileHandle.h"
#include "NREHandle.h"
using namespace std;
const unsigned MIN_SIZE=4096;
const unsigned MAX_SIZE=16384;
const unsigned AVG_SIZE=8912;
struct NREThread{

	 NREThread(const string& name, const vector<string>& file,
				boost::shared_ptr<BloomFilter> BF,
				boost::shared_ptr<LRUCache<string,string> > Cache)
	 {

		 boost::shared_ptr<NRE_Handle> Nptr
		 (new NRE_Handle(name,file,BF,Cache,MIN_SIZE,AVG_SIZE,MAX_SIZE));
		 Nptr->start();
	 };
	 void operator( )( ){};
};
int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("usage: %s path\n", argv[0]);
		return -1;
	}
	boost::thread_group tg;
	FileHandle fh(argv[1]);
	vector<string> keyVec;
	vector<string> valueVec;
	vector<string>::iterator namePos;
	fh.getKeySuffix(keyVec);
	//boost::shared_ptr<BloomFilter> bp(new BloomFilter(128000000,6));
	boost::shared_ptr<LRUCache<string,string> > cache(new LRUCache<string,string>(4096));
	//unsigned int keyCounts=fh.SuffixCounts();
	//NRE_Handle* nh[keyCounts];
    unsigned icounts=0;
	vector<string>::iterator keyPos=keyVec.begin();
	//cout << "The vector size is is " <<*keyPos<<endl;

	while(keyPos!=keyVec.end()) {
		valueVec.clear();
		fh.getFileNamefromSuffix(*keyPos, valueVec);
//		boost::thread thrd_t (NREThread(*keyPos,valueVec,bp,cache));
//		thrd_t.join();
		tg.create_thread(NREThread(*keyPos,valueVec,bp,cache));
//		NRE_Handle nh(*keyPos,valueVec,bp,cache,MIN_SIZE,AVG_SIZE,MAX_SIZE);
//		nh.start();
		++icounts;
		++keyPos;
	}
	tg.join_all();
	cout<<"the main thread is end"<<endl;
//	while(counts!=0)
//	{
//		delete[] nh[counts--];
//	}
//		boost::thread thrd1(&chunk_file, (*valuePos).c_str());
//		boost::thread thrd2(&showing);
//		thrd1.join();
//		thrd2.join();
}
