/*
 * FileHandle.h
 *
 *  Created on: 2013-1-13
 *      Author: liuyi
 */

#ifndef FILEHANDLE_H_
#define FILEHANDLE_H_
#include <map>
#include <string>
#include <vector>
using namespace std;
class FileHandle {
public:
	FileHandle(const string& dirPath):srcpath(dirPath){travel_dir(srcpath);};
	unsigned int SuffixCounts();
    unsigned int getKeySuffix(vector<string> &vec);
	unsigned int getFileNamefromSuffix(const string& suffix,vector<string> &vec);
	void cleanContainer()
	{
		SuffixContainer.clear();
	};
private:
	FileHandle(const FileHandle&);
	FileHandle& operator=(const FileHandle&);
	inline bool isEmpty()
	{
		return SuffixContainer.empty();
	};
	//inline const char* getSuffixName(const char* FileName);
	inline bool isInSuffixContainer(const string& SuffixName)
	{
		return (SuffixContainer.find(SuffixName)!=SuffixContainer.end());
	};
	int travel_dir(const string& path); //travel the files;
private:
	const string srcpath;
	typedef map<const string,vector<string> > FileContainer;
	FileContainer SuffixContainer;
};

#endif /* FILEHANDLE_H_ */
