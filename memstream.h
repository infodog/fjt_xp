#ifndef _MEM_STREAM_H
#define _MEM_STREAM_H

#include "config.h"

#define MYPAGESIZE 1020


typedef struct mem_buffer {
	char m_chData[MYPAGESIZE];
	struct mem_buffer *m_pmbNext;
} mem_buffer;

typedef struct memstream {
	pool *m_pp;
	mem_buffer *m_pmbHead;
	unsigned int m_iOffset;	
	mem_buffer *m_pmbTail;
	unsigned int m_iLeft;
	unsigned int m_iFree;
	unsigned int m_iTotal;	
} memstream;

memstream* create_memstream(pool *p);
int memstream_write(memstream *pms, char *pdata, unsigned int nsize);
int memstream_read(memstream *pms, char *pbuff, unsigned int nsize);
int memstream_foreach(memstream *pms, char **pbuff);
int memstream_copy(memstream *dms, memstream *sms);
unsigned int get_memstream_size(memstream *pms);
unsigned int get_memstream_datasize(memstream *pms);
void memstream_clear(memstream *pms);
void delete_memstream(memstream *pms);


#endif
