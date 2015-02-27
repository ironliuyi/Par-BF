/*
 * FileHandle.cpp
 *
 *  Created on: 2013-1-13
 *      Author: liuyi
 */

#include "FileHandle.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <vector>
unsigned int FileHandle::SuffixCounts()
{
	return SuffixContainer.size();
}

unsigned int FileHandle::getKeySuffix(vector<string>& vec)
{
	FileContainer::iterator pos;
	if(isEmpty())
		return 0;
	pos=SuffixContainer.begin();
	while(pos!=SuffixContainer.end())
	{
		vec.push_back(pos->first);
		++pos;
	}
	return vec.size();
}
unsigned int FileHandle:: getFileNamefromSuffix(const string& suffix,vector<string>& vec)
{
	if(!isInSuffixContainer(suffix))
		return 0;
	FileContainer::iterator pos;
	pos = SuffixContainer.find(suffix);
	if(pos != SuffixContainer.end())
		vec=pos->second;
	return vec.size();
}
int FileHandle::travel_dir(const string& path)
{
	FileContainer::iterator pos;
   	DIR *dHandle;
   	struct dirent *file;
   	struct stat sb;
   	const char* extname;
    char fullpath[FILENAME_MAX];
    static unsigned int counts;
   	if(!(dHandle=opendir(path.c_str())))
   	{
   		printf("error opendir %s!!!\n",path.c_str());
   		return -1;
   	}
   	while((file=readdir(dHandle)) != NULL)
   	{
   		//把当前目录.，上一级目录.. 以及隐藏文件去掉，避免死循环
   		if(0 == strncmp(file->d_name,".",1) ||
   		   0 == strncmp(file->d_name,".",1))
   			continue;
   		sprintf(fullpath,"%s/%s",path.c_str(),file->d_name);
   		if(lstat(fullpath,&sb)>=0
   		   && S_ISDIR(sb.st_mode))
   		{
   			travel_dir(fullpath);
   		}
   		else
   		{
   			extname=strrchr(file->d_name,'.');
   			++counts;
   			if(extname == NULL)
   				continue;
   			pos=SuffixContainer.find(extname);
   			if(pos == SuffixContainer.end())
   			{
   				vector<string> FileItem;
   				FileItem.push_back(fullpath);
   				SuffixContainer.insert(std::make_pair(extname,FileItem));
   			}
   			else
   			{
   				(pos->second).push_back(fullpath);
   			}
   		}
   	}
   	closedir(dHandle);
   	return counts;
}

