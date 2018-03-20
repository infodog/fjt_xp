#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#ifdef USE_INTERNAL_MM
#include "mm.h"
#endif

#include "config.h"
#include "memstream.h"


void apr_pfree(pool *p, void *pbuff)
{
	free(pbuff);
}

memstream* create_memstream(pool *p)
{
	memstream *newms = NULL;
	
	newms = apr_palloc(p, LOGALLOCSIZE(sizeof(memstream)));
	if (newms == NULL) {
		return NULL;
	}
	newms->m_pp = p;
	newms->m_iLeft = MYPAGESIZE;
	newms->m_iFree = 0;
	newms->m_iTotal = 0;
	newms->m_iOffset = 0;
	newms->m_pmbTail = NULL;	
	newms->m_pmbHead = NULL;
	return newms;
}

unsigned int get_memstream_size(memstream *pms)
{
	return pms->m_iTotal;
}

unsigned int get_memstream_datasize(memstream *pms)
{
	return pms->m_iTotal - pms->m_iFree;
}

void memstream_clear(memstream *pms)
{
	pms->m_iOffset = 0;
	pms->m_iLeft = MYPAGESIZE;
	pms->m_iFree = pms->m_iTotal;	
	pms->m_pmbTail = pms->m_pmbHead;
}

int memstream_write(memstream *pms, char *pdata, unsigned int nsize)
{
	int no = 0;
	mem_buffer *tmpmb = NULL, *bakmb = NULL;
	
	while (pms->m_iFree < nsize) {
		tmpmb = apr_palloc(pms->m_pp, LOGALLOCSIZE(sizeof(mem_buffer)));
		if (tmpmb == NULL) {
			return 0;
		}
		if (pms->m_pmbTail == NULL) {
			pms->m_pmbHead = pms->m_pmbTail = tmpmb;
			pms->m_pmbTail->m_pmbNext = pms->m_pmbHead;
			pms->m_pmbHead->m_pmbNext = pms->m_pmbTail;
			pms->m_iLeft = MYPAGESIZE;
		}
		else {
			tmpmb->m_pmbNext = pms->m_pmbTail->m_pmbNext;
			pms->m_pmbTail->m_pmbNext = tmpmb;
			if (bakmb == NULL && pms->m_pmbHead == pms->m_pmbTail
				&& pms->m_iLeft - pms->m_iFree == MYPAGESIZE - pms->m_iOffset) {
				bakmb = tmpmb;
				memcpy(bakmb->m_chData + pms->m_iOffset, 
					pms->m_pmbHead->m_chData + pms->m_iOffset, MYPAGESIZE - pms->m_iOffset);
				pms->m_pmbHead = bakmb;
			}
		}
		pms->m_iFree += MYPAGESIZE;
		pms->m_iTotal += MYPAGESIZE;
	}
	
	while (nsize > 0) {		
		if (pms->m_iLeft > nsize) {
			memcpy(pms->m_pmbTail->m_chData + MYPAGESIZE - pms->m_iLeft, pdata, nsize);					
			no += nsize;
			pms->m_iLeft -= nsize;
			pms->m_iFree -= nsize;					
			return no;
		}
		else {
			memcpy(pms->m_pmbTail->m_chData + MYPAGESIZE - pms->m_iLeft, pdata, pms->m_iLeft);
			no += pms->m_iLeft;
			nsize -= pms->m_iLeft;	
			pdata += pms->m_iLeft;
			pms->m_iFree -= pms->m_iLeft;
			pms->m_iLeft = MYPAGESIZE;
			pms->m_pmbTail = pms->m_pmbTail->m_pmbNext;					
		}
	}
	
	return no;
}

int memstream_read(memstream *pms, char *pbuff, unsigned int nsize)
{
	unsigned int no = 0, num = 0;
	
	while (nsize > 0 && pms->m_iTotal - pms->m_iFree > 0) {
		num = MYPAGESIZE - pms->m_iOffset;
		if (num > pms->m_iTotal - pms->m_iFree) {
			num = pms->m_iTotal - pms->m_iFree;			
		}
		if (num > nsize) {
			memcpy(pbuff, pms->m_pmbHead->m_chData + pms->m_iOffset, nsize);
			no += nsize;
			pms->m_iFree += nsize;
			pms->m_iOffset += nsize;
			return no;
		}
		else {
			memcpy(pbuff, pms->m_pmbHead->m_chData + pms->m_iOffset, num);
			no += num;
			nsize -= num;
			pbuff += num;
			pms->m_iFree += num;
			pms->m_pmbHead = pms->m_pmbHead->m_pmbNext;
			pms->m_iOffset = 0;
		}
	}
	
	if (pms->m_iTotal == pms->m_iFree) {
		pms->m_pmbTail = pms->m_pmbHead;		
		pms->m_iOffset = 0;
		pms->m_iLeft = MYPAGESIZE;
	}
	
	return no;
}

int memstream_foreach(memstream *pms, char **pbuff)
{
	unsigned int num = 0;
	
	*pbuff = NULL;
	if (pms->m_iTotal - pms->m_iFree > 0) {
		num = MYPAGESIZE - pms->m_iOffset;
		if (num > pms->m_iTotal - pms->m_iFree) {
			num = pms->m_iTotal - pms->m_iFree;			
		}
		*pbuff = pms->m_pmbHead->m_chData + pms->m_iOffset;
		pms->m_iFree += num;
		pms->m_pmbHead = pms->m_pmbHead->m_pmbNext;
		pms->m_iOffset = 0;	    
		if (pms->m_iTotal == pms->m_iFree) {
			pms->m_pmbTail = pms->m_pmbHead;		
			pms->m_iOffset = 0;
			pms->m_iLeft = MYPAGESIZE;
		}
	}
	
	return num;
}

int memstream_copy(memstream *dms, memstream *sms)
{
	int num = 0;
	int total = 0;
	char buff[MYPAGESIZE * 2];
	
	while ((num = memstream_read(sms, buff, sizeof(buff))) > 0) {
		memstream_write(dms, buff, num);
		total += num;
	}
	return total;
}

void delete_memstream(memstream *pms)
{
	if (pms->m_pmbTail != NULL) {
		mem_buffer *tmpms = pms->m_pmbTail->m_pmbNext;		
		while (tmpms != pms->m_pmbTail) {
			mem_buffer *tms = tmpms->m_pmbNext;
			apr_pfree(pms->m_pp, tmpms);
			tmpms = tms;
		}
		apr_pfree(pms->m_pp, tmpms);
	}	
}

