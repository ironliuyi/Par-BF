/*
 * chunker.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: ubuntu
 */

#include "chunker.h"
#include "MurmurHash64.h"

#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <libhashish.h>
#include <assert.h>
#include <math.h>

#define FINGERPRINT_PT  0xbfe6b8a5bf378d83LL
#define BREAKMARK_VALUE 0x12

const string FILE_END_TOKEN = "FILE_CHUNKING_END";
const string PROGRESS_END_TOKEN = "PROCESSING_END";

chunker::chunker(boost::shared_ptr<bounded_buffer<chunk> > sp,
		unsigned mincs,unsigned avgcs,unsigned maxcs):
min_chunk_size(mincs),avg_chunk_size(avgcs),max_chunk_size(maxcs),
min_size_suppress(0), max_size_suppress(0),
_w(FINGERPRINT_PT)
{
  buffer=sp;
  chunkmask= (0x1 << ((int)ceil(log2(avg_chunk_size))))-1; //chunk mask 计算公式
  _last_pos = 0;
  _cur_pos = 0;
  _w.reset();
  _hbuf_cursor = 0;
  _hbuf = new unsigned char[32768];
  _hbuf_size = 32768;
}
void chunker::Initialize()
{
	min_size_suppress=0;
	max_size_suppress=0;
	_last_pos = 0;
	_cur_pos = 0;
	_w.reset();
	_hbuf_cursor = 0;
	delete[] _hbuf;
	_hbuf = new unsigned char[32768];
	_hbuf_size = 32768;
}

chunker::~chunker()
{
	delete [] _hbuf;
}

void
chunker::handle_hash(const unsigned char *data, size_t size)
{
  if (size > 0) {
    while (_hbuf_cursor+size > _hbuf_size) {
      unsigned char *nb = new unsigned char[_hbuf_size*2];
      if(NULL==nb)
    	  cerr<<"Memory Allocation Error!"<<endl;
      memmove(nb, _hbuf, _hbuf_cursor);
      _hbuf_size *= 2;
      delete[] _hbuf;
      _hbuf = nb;
    }
    memmove(_hbuf+_hbuf_cursor, data, size);
    _hbuf_cursor += size;
  }
}

void
chunker::stop()
{
  if (_cur_pos != _last_pos) {
	string data(_hbuf,_hbuf+_cur_pos-_last_pos);
	KEY_TYPE fp=MurmurHash64(data.c_str(),data.length());
    chunk c(fp,_last_pos,_cur_pos-_last_pos,data);
    assert(_cur_pos-_last_pos == _hbuf_cursor);
    _hbuf_cursor = 0;
    buffer->push_front(c);
  }
  chunk end(0,0,0,FILE_END_TOKEN);
  buffer->push_front(end);
}


void
chunker::chunk_data(const unsigned char *data, size_t size)
{
  uint64_t f_break = 0;
  size_t start_i = 0;
  for (size_t i=0; i<size; i++, _cur_pos++) {
	f_break = _w.slide8 (data[i]); //计算break_value
    size_t cs = _cur_pos - _last_pos; //数据大小
    if (((f_break & chunkmask) % avg_chunk_size) == BREAKMARK_VALUE && cs < min_chunk_size)
      min_size_suppress++;
    else if (cs == max_chunk_size)
      max_size_suppress++;
    if ((((f_break & chunkmask) % avg_chunk_size) == BREAKMARK_VALUE && cs >= min_chunk_size)
	|| cs >= max_chunk_size) {
      _w.reset();
      if (i-start_i > 0)
	  handle_hash(data+start_i, i-start_i);
      string data(_hbuf,_hbuf+cs);
      KEY_TYPE fp=MurmurHash64(data.c_str(),data.length());
     // cout<<"the fp value is"<<fp<<endl;
      chunk c(fp,_last_pos,cs,data);
      if (_hbuf_cursor != cs)
	  cerr << "_hbuf_cursor = " << _hbuf_cursor << ", cs = " << cs << "\n";
      assert(_hbuf_cursor == cs);
      _hbuf_cursor = 0;
       buffer->push_front(c);
      _last_pos = _cur_pos;
      start_i = i;
    }

  }
  handle_hash(data+start_i, size-start_i);
}

void
chunker::showData()
{
	chunk receiver;
	do
	{
		buffer->pop_back(&receiver);
		cout<<"##############BEGIN#######################"<<endl;
		cout<<"the received chunk is here"<<endl;
		cout<<"the chunk value is: "<<receiver.data<<endl;
		cout<<"the length is: "<<receiver.size<<endl;
		cout<<"##############END#########################"<<endl;
	}while(strcmp(receiver.data.c_str(),FILE_END_TOKEN.c_str())!=0 &&
		   receiver.hash_value!=0 );
}

void
chunker::chunk_finished()
{
	chunk progress_end(0,0,0,PROGRESS_END_TOKEN);
	buffer->push_front(progress_end);
}
