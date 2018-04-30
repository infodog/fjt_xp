#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "config.h"
#include "baseutil.h"
#include "convert.h"
#include "apr_base64.h"

#define API_EXPORT(x) x
#ifdef CHARSET_EBCDIC
#include "ebcdic.h"
#endif

char g_UrlDecodeTable[256];

static const unsigned char pr2six[256] =
{
#ifndef CHARSET_EBCDIC
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 63, 64, 64, 
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 
		64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64, 
		64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
#else
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 63, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 64, 64, 64, 64, 64, 64, 
		64, 35, 36, 37, 38, 39, 40, 41, 42, 43, 64, 64, 64, 64, 64, 64, 
		64, 64, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, 64, 
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
		64,  0,  1,  2,  3,  4,  5,  6,  7,  8, 64, 64, 64, 64, 64, 64, 
		64,  9, 10, 11, 12, 13, 14, 15, 16, 17, 64, 64, 64, 64, 64, 64, 
		64, 64, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64, 64, 
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64
#endif
}; 

API_EXPORT(int) if_xbase64decode_len(const char *bufcoded)
{
	int nbytesdecoded; 
	register const unsigned char *bufin; 
	register int nprbytes; 
	
	bufin = (const unsigned char *) bufcoded; 
	while (pr2six[*(bufin++)] <= 63); 
	
	nprbytes = (bufin - (const unsigned char *) bufcoded) - 1; 
	nbytesdecoded = ((nprbytes + 3) / 4) * 3; 
	
	return nbytesdecoded + 1; 
}

API_EXPORT(int) if_xbase64decode_binary(unsigned char *bufplain, 
										const char *bufcoded)
{
	int nbytesdecoded; 
	register const unsigned char *bufin; 
	register unsigned char *bufout; 
	register int nprbytes; 
	
	bufin = (const unsigned char *) bufcoded; 
	while (pr2six[*(bufin++)] <= 63); 
	nprbytes = (bufin - (const unsigned char *) bufcoded) - 1; 
	nbytesdecoded = ((nprbytes + 3) / 4) * 3; 
	
	bufout = (unsigned char *) bufplain; 
	bufin = (const unsigned char *) bufcoded; 
	
	while (nprbytes > 4) {
		*(bufout++) =
			(unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4); 
		*(bufout++) =
			(unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2); 
		*(bufout++) =
			(unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]); 
		bufin += 4; 
		nprbytes -= 4; 
	}
	
	if (nprbytes > 1) {
		*(bufout++) =
			(unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4); 
	}
	if (nprbytes > 2) {
		*(bufout++) =
			(unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2); 
	}
	if (nprbytes > 3) {
		*(bufout++) =
			(unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]); 
	}
	
	nbytesdecoded -= (4 - nprbytes) & 3; 
	return nbytesdecoded; 
}

int if_xbase64decode_ex(unsigned char *bufplain, 
						const char *bufcoded, int nprbytes)
{
	int nbytesdecoded; 
	register const unsigned char *bufin; 
	register unsigned char *bufout;
	
	bufin = (const unsigned char *) bufcoded;         
	while (pr2six[*bufin] <= 63 && bufin < (unsigned char*)bufcoded + nprbytes) {
		++bufin;
	}
	nprbytes = bufin - (unsigned char*)bufcoded;
	
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;     
	bufout = (unsigned char *) bufplain; 
	bufin = (const unsigned char *) bufcoded;     
	while (nprbytes > 4) {
		*(bufout++) =
			(unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4); 
		*(bufout++) =
			(unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2); 
		*(bufout++) =
			(unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]); 
		bufin += 4; 
		nprbytes -= 4; 
	}   
	if (nprbytes > 1) {
		*(bufout++) =
			(unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4); 
	}
	if (nprbytes > 2) {
		*(bufout++) =
			(unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2); 
	}
	if (nprbytes > 3) {
		*(bufout++) =
			(unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]); 
	}    
	nbytesdecoded -= (4 - nprbytes) & 3; 
	
	return nbytesdecoded; 
}

int UnBase64Block(char *inbuf, int size)
{
	int len = 0;
	char *ptr = NULL;
	char *pbak = NULL;
	char *puse = inbuf;
	char *pend = inbuf + size;
	
	while (puse < pend) {
		ptr = strnistr(puse, "-base", pend - puse);
		if (ptr == NULL) {
			break;
		}
		if (pbak == NULL) {
			pbak = ptr;
		}
		else {
			memmove(pbak, puse, ptr - puse);
			pbak += ptr - puse;
		}
		ptr += 5;
		puse = ptr;
		while (*ptr != '-' && ptr < pend) {
			++ptr;
		}
		len = atoi(puse);
		++ptr;
		if (len < 1 || ptr + len > pend) {
			memcpy(pbak, "-base", 5);
			pbak += 5;
		}
		else {            
			puse = ptr + len;            
			len = if_xbase64decode_ex((unsigned char*)pbak, ptr, len);
			pbak += len;
		}
	}
	if (pbak == NULL) {
		return size;
	}
	if (puse < pend) {
		memmove(pbak, puse, pend - puse);
		pbak += pend - puse;
	}
	
	return pbak - inbuf;
}

int ResetCode7E(char *url, int len)
{
	char *ptr = url;
	char *pbk = url;
	char *ped = url + len;
	
	if (strnistr(url, "-base", len) == NULL) {
		return len;
	}    
	
	while ((ptr < ped) && (*ptr != '\0')) {
		if (*ptr == '%' && *(ptr + 1) == '7' 
			&& (*(ptr + 2) == 'e' || *(ptr + 2) == 'E')) {
			*pbk++ = '~';
			ptr += 3;
		}
		else {
			*pbk++ = *ptr++;
		}
	}
	
	return pbk - url;    
}

API_EXPORT(int) if_xbase64decode(char *bufplain, const char *bufcoded)
{
#ifdef CHARSET_EBCDIC
	int i; 
#endif
	int len; 
	
	len = if_xbase64decode_binary((unsigned char *) bufplain, bufcoded); 
#ifdef CHARSET_EBCDIC
	for (i = 0; i < len; i++)
		bufplain[i] = os_toebcdic[bufplain[i]]; 
#endif
	bufplain[len] = '\0'; 
	return len; 
}

static const char basis_64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-"; 

API_EXPORT(int) if_xbase64encode_len(int len)
{
	return ((len + 2) / 3 * 4) + 1; 
}

API_EXPORT(int) if_xbase64encode_binary(char *encoded, 
										const unsigned char *string, int len)
{
	int i; 
	char *p; 
	
	p = encoded; 
	for (i = 0; i < len - 2; i += 3) {
		*p++ = basis_64[(string[i] >> 2) & 0x3F]; 
		*p++ = basis_64[((string[i] & 0x3) << 4) |
			((int) (string[i + 1] & 0xF0) >> 4)]; 
		*p++ = basis_64[((string[i + 1] & 0xF) << 2) |
			((int) (string[i + 2] & 0xC0) >> 6)]; 
		*p++ = basis_64[string[i + 2] & 0x3F]; 
	}
	if (i < len) {
		*p++ = basis_64[(string[i] >> 2) & 0x3F]; 
		if (i == (len - 1)) {
			*p++ = basis_64[((string[i] & 0x3) << 4)]; 
			*p++ = '~'; 
		}
		else {
			*p++ = basis_64[((string[i] & 0x3) << 4) |
				((int) (string[i + 1] & 0xF0) >> 4)]; 
			*p++ = basis_64[((string[i + 1] & 0xF) << 2)]; 
		}
		*p++ = '~'; 
	}
	
	*p++ = '\0'; 
	return p - encoded; 
}

API_EXPORT(int) if_xbase64encode(char *encoded, const char *string, int len)
{
#ifndef CHARSET_EBCDIC
	return if_xbase64encode_binary(encoded, (const unsigned char *) string, len); 
#else	
	int i; 
	char *p; 
	
	p = encoded; 
	for (i = 0; i < len - 2; i += 3) {
		*p++ = basis_64[(os_toascii[string[i]] >> 2) & 0x3F]; 
		*p++ = basis_64[((os_toascii[string[i]] & 0x3) << 4) |
			((int) (os_toascii[string[i + 1]] & 0xF0) >> 4)]; 
		*p++ = basis_64[((os_toascii[string[i + 1]] & 0xF) << 2) |
			((int) (os_toascii[string[i + 2]] & 0xC0) >> 6)]; 
		*p++ = basis_64[os_toascii[string[i + 2]] & 0x3F]; 
	}
	if (i < len) {
		*p++ = basis_64[(os_toascii[string[i]] >> 2) & 0x3F]; 
		if (i == (len - 1)) {
			*p++ = basis_64[((os_toascii[string[i]] & 0x3) << 4)]; 
			*p++ = '~'; 
		}
		else {
			*p++ = basis_64[((os_toascii[string[i]] & 0x3) << 4) |
				((int) (os_toascii[string[i + 1]] & 0xF0) >> 4)]; 
			*p++ = basis_64[((os_toascii[string[i + 1]] & 0xF) << 2)]; 
		}
		*p++ = '~'; 
	}
	
	*p++ = '\0'; 
	return p - encoded; 
#endif
}

char* GetBuffArray(ConvertCtx *pctx, int index, int prefix_len)
{
	static int vec = sizeof(pctx->pBuffArray) / sizeof(pctx->pBuffArray[0]); 
	
	if (index >=0 && index < vec) {
		if (pctx->pBuffArray[index] == NULL) {
			pctx->pBuffArray[index] = apr_palloc(pctx->p, LOGALLOCSIZE(pctx->nBuffArraySize)); 			
		}
		return pctx->pBuffArray[index] + prefix_len;
	}
	else {
		char *new_buff = apr_palloc(pctx->p, LOGALLOCSIZE(pctx->nBuffArraySize));
		return new_buff + prefix_len;
	}
}

int UrlDecodeHZ(char *inbuf, int insize, char *outbuf, int *outsize)
{
	unsigned char *p, *pe; 
	unsigned char *pout; 
	pout = outbuf; 
	p = inbuf; 
	pe = inbuf + insize - 1;
	
	while(p <= pe) {
		if (*p == '%') {
			unsigned char c1, c2; 
			c1 = g_UrlDecodeTable[*(p+1)]; 
			if (*(p+1) == 'u') {
				*pout++ = *p++;
				continue;
			}
			c2 = g_UrlDecodeTable[*(p+2)]; 
			*pout = (c1 << 4) + c2; 
			pout++; 
			p += 3; 
		}
		else {
			*pout = *p; 
			p++;
			pout++; 
		}        
	}
	
	*outsize = (unsigned int)pout - (unsigned int)outbuf ; 
	return 1; 
}


char* UnConvertUrl(ConvertCtx *pctx, pool *apool, config *pconfig, char *url)
{
	int nsize = 0;
	int outlen = 0;
	int outlen1 = 0;
	char *p = NULL;
	char *outbuf = NULL;
	char *realoutbuf = NULL;
	char tembuf[2560];
	memstream *pstream = NULL;
	
	nsize = strlen(url);    
	if (pconfig->m_iShouldChangeUrlInServer == 0 || nsize > 2048) {
		return url; 
	}
	
	if (pconfig->m_iOutputSameEncode == 1) {
		outbuf = apr_palloc(apool, LOGALLOCSIZE(nsize * 3 + 128));
		UrlDecodeHZ(url, nsize, tembuf, &outlen);
		outlen = ConvertToUnicodeExt(pconfig->m_iFromEncode, FFFE, 0, tembuf, outlen, outbuf);
		if (pconfig->m_iFromEncode == ENCODE_GB2312) {
			outlen = ConvertUnicodeExt(UNICODE_BIG5, UNICODE_GB2312, pconfig->m_iConvertWord, outbuf, outlen, tembuf,sizeof(tembuf));
		}
		else {
			outlen = ConvertUnicodeExt(UNICODE_GB2312, UNICODE_BIG5, pconfig->m_iConvertWord, outbuf, outlen, tembuf,sizeof(tembuf));
		}
		outlen = ConvertFromUnicodeEx(pconfig->m_iFromEncode, FFFE, tembuf, outlen, outbuf);
		outbuf[outlen] = '\0';
		return outbuf;
	}
	
	/* // ����ط��������ˣ� ��Ϊ���� unconvert_orig(...) �����������ظ�����
	if (pconfig->m_iSendURLsAsUTF8 == 1) {		
	outbuf = apr_palloc(apool, LOGALLOCSIZE(nsize * 3 + 128));
	UrlDecodeHZ(url, nsize, outbuf, &outlen);
	if (pctx->nUrlEncode == ENCODE_UNKNOW) {
	outlen = ConvertFromUTF8Ex(outbuf, outlen, pconfig->m_iToEncode, pctx);			
	}
	pstream = create_memstream(apool);
	unconvert_orig(pconfig->m_usetable, outbuf, outlen, pstream, pconfig, pctx); 
	outlen = memstream_read(pstream, outbuf, LOGALLOCSIZE(nsize * 3 + 128));
	outbuf[outlen] = '\0';
	return outbuf;
	}
	*/
	
	p = strchr(url, '?'); 	    	
	if (p == NULL) { // WPF 2010-10-26 
		p = strchr(url, '%'); 	    
		if (p == NULL || p <= url) {
			if (pconfig->m_iUnConvertWholeUrl != 1 || (p = strchr(url, '/')) == NULL) {
				return url; 
			}
		}
		--p;
	}
	
	p++;
	realoutbuf = apr_palloc(apool, LOGALLOCSIZE(nsize * 3 + 128));
	outbuf = realoutbuf + (p - url);
	pstream = create_memstream(apool);
	
	if (pconfig->m_iIgnoreUrlEncode == 1) {
		unconvert_orig(pconfig->m_usetable, p, url + nsize - p, pstream, pconfig, pctx);
		outlen = memstream_read(pstream, outbuf, LOGALLOCSIZE(nsize * 3 + 128));
	} 
	else {
		int p25 = UrlDecode(pconfig, p, url + nsize - p, outbuf, &outlen); 	
		
		unconvert_orig(pconfig->m_usetable, outbuf, outlen, pstream, pconfig, pctx); 
		outlen1 = memstream_read(pstream, tembuf, sizeof(tembuf)); 
		if (outlen1 == outlen && memcmp(tembuf, outbuf, outlen) == 0) {
			return url; 
		}
		UrlEncode(tembuf, outlen1, outbuf, &outlen); 
		
		if (p25 == 2) { // '%' --> '%25'
			int i = 0;
			char *ptb = tembuf;
			
			for (i = 0; i < outlen; i++) {
				if (outbuf[i] == '%') {
					memcpy(ptb, "%25", 3);
					ptb += 3;
				}
				else {
					*ptb++ = outbuf[i];
				}
			}
			if (ptb > tembuf) {
				outlen = ptb - tembuf;
				*ptb = '\0';
				strcpy(outbuf, tembuf);
			}
		}
	}
	
	outbuf[outlen] = '\0'; 	
	memcpy(realoutbuf, url, p - url);
	return realoutbuf;
}

char* ConvertUrl_orig(ConvertCtx *pctx, pool *apool, config *pconfig, char *url)
{
	config excon;
	memcpy(&excon, pconfig, sizeof(config));
	excon.m_iFromEncode = pconfig->m_iToEncode;
	excon.m_iToEncode   = pconfig->m_iFromEncode;
	excon.m_usetable    = pconfig->m_negtable;
	return UnConvertUrl(pctx, apool, &excon, url);
}

char* ConvertUrl(ConvertCtx *pctx, pool *apool, config *pconfig, char *url)
{
	int nsize = 0;
	int outlen = 0;
	int outlen1 = 0;
	char *p = NULL;
	char *outbuf = NULL;
	char *realoutbuf = NULL;
	char tembuf[2560];
	memstream *pstream = NULL;
	
	nsize = strlen(url);    
	if (pconfig->m_iShouldChangeUrlInServer == 0 || (nsize * 2) > (pctx->nBuffArraySize - 256)) {
		return url; 
	}
	p = strchr(url, '?'); 	    
	if (p == NULL) {
		return url; 
	}
	
	p++;
	realoutbuf = GetBuffArray(pctx, 1, 0); 
	outbuf = realoutbuf + (p - url);
	pstream = create_memstream(apool);
	
	if (pconfig->m_iIgnoreUrlEncode == 1) {
		convert(pconfig->m_usetable, p, url + nsize - p, pstream, pconfig, pctx);
		outlen = memstream_read(pstream, outbuf, pctx->nBuffArraySize);
	} 
	else {
		UrlDecode(pconfig, p, url + nsize - p, outbuf, &outlen); 	
		convert(pconfig->m_usetable, outbuf, outlen, pstream, pconfig, pctx); 
		outlen1 = memstream_read(pstream, tembuf, sizeof(tembuf)); 
		if (outlen1 == outlen && memcmp(tembuf, outbuf, outlen) == 0) {
			return url; 
		}
		UrlEncode(tembuf, outlen1, outbuf, &outlen); 
	}
	
	outbuf[outlen] = '\0'; 	
	memcpy(realoutbuf, url, p - url);
	return realoutbuf;
}

void InitUrlEncodeTable()
{
	memset(g_UrlDecodeTable, 0, sizeof g_UrlDecodeTable); 
	g_UrlDecodeTable['1'] = 1; 
	g_UrlDecodeTable['2'] = 2; 
	g_UrlDecodeTable['3'] = 3; 
	g_UrlDecodeTable['4'] = 4; 
	g_UrlDecodeTable['5'] = 5; 
	g_UrlDecodeTable['6'] = 6; 
	g_UrlDecodeTable['7'] = 7; 
	g_UrlDecodeTable['8'] = 8; 
	g_UrlDecodeTable['9'] = 9; 
	g_UrlDecodeTable['0'] = 0; 
	
	g_UrlDecodeTable['a'] = 10; 
	g_UrlDecodeTable['b'] = 11; 
	g_UrlDecodeTable['c'] = 12; 
	g_UrlDecodeTable['d'] = 13; 
	g_UrlDecodeTable['e'] = 14; 
	g_UrlDecodeTable['f'] = 15; 
	
	g_UrlDecodeTable['A'] = 10; 
	g_UrlDecodeTable['B'] = 11; 
	g_UrlDecodeTable['C'] = 12; 
	g_UrlDecodeTable['D'] = 13; 
	g_UrlDecodeTable['E'] = 14; 
	g_UrlDecodeTable['F'] = 15; 
}

unsigned char g_pcxSafeTable[256]; 
#define IsSafe(c) (g_pcxSafeTable[c])

void InitSafeTable()
{
	int i; 
	memset(g_pcxSafeTable, 0, 256); 
	for(i='a'; i<='z'; i++)
	{
		g_pcxSafeTable[i]=1; 
	}
	for(i='A'; i<='Z'; i++)
	{
		g_pcxSafeTable[i]=1; 
	}
	for(i='0'; i<='9'; i++)
	{
		g_pcxSafeTable[i]=1; 
	}
	g_pcxSafeTable['_']=1; 
	g_pcxSafeTable['.']=1; 
	g_pcxSafeTable['-']=1; 
	g_pcxSafeTable['*']=1; 
	g_pcxSafeTable['@']=1; 
	g_pcxSafeTable[':']=1; 
	g_pcxSafeTable['/']=1; 
	
}

int UrlEncode(char *inbuf, int insize, char *outbuf, int *outsize)
{
	unsigned char *p, *pe; 
	unsigned char *pout; 
	pout = outbuf; 
	p = inbuf; 
	pe = inbuf + insize -1; 
	while(p <= pe)
	{
		if (*p == 2)
		{
			*pout = '='; 
			pout++; 
		}
		else if (*p == 1)
		{
			*pout = '&'; 
			pout++; 
		}
		else if (*p == ' ')
		{
			*pout = '+'; 
			pout++; 
		}
		else if (IsSafe(*p))
		{
			*pout = *p; 
			pout++; 
		}
		else
		{
			int c1, c2; 
			c1 = *p >> 4; 
			c2 = *p & 0x0F; 
			*pout = '%'; pout++; 
			if(c1 > 9)
			{
				*pout = (c1 - 10) + 'A'; 
				pout++; 
			}
			else
			{
				*pout = c1 + '0'; 
				pout++; 
			}
			if(c2 > 9)
			{
				*pout = (c2 - 10) + 'A'; 
				pout++; 
			}
			else
			{
				*pout = c2 + '0'; 
				pout++; 
			}
		}
		p++; 
	}
	*outsize = (unsigned int)pout - (unsigned int)outbuf ; 
	return 1; 
}

int UrlRepair(char *inbuf, int nsize)
{
	int i; 
	for(i=0; i<nsize; i++)
	{
		if (inbuf[i] == 2)
		{
			inbuf[i] = '='; 
		}
		else if (inbuf[i] == 1)
		{
			inbuf[i] = '&'; 
		}
		else if (inbuf[i] == ' ')
		{
			inbuf[i] = '+'; 
			
		}
	}
	return nsize; 
}

int DataEncode(char *inbuf, int insize, char *outbuf, int *outsize)
{
	unsigned char *p, *pe; 
	unsigned char *pout; 
	pout = outbuf; 
	p = inbuf; 
	pe = inbuf + insize -1; 
	while(p <= pe)
	{
		if (*p == 2)
		{
			*pout = '='; 
			pout++; 
		}
		else if (*p == 1)
		{
			*pout = '&'; 
			pout++; 
		}
		else if (*p == ' ')
		{
			*pout = '+'; 
			pout++; 
		}
		else if (IsSafe(*p) || *p == '%')
		{
			*pout = *p; 
			pout++; 
		}
		else
		{
			int c1, c2; 
			c1 = *p >> 4; 
			c2 = *p & 0x0F; 
			*pout = '%'; pout++; 
			if(c1 > 9)
			{
				*pout = (c1 - 10) + 'A'; 
				pout++; 
			}
			else
			{
				*pout = c1 + '0'; 
				pout++; 
			}
			if(c2 > 9)
			{
				*pout = (c2 - 10) + 'A'; 
				pout++; 
			}
			else
			{
				*pout = c2 + '0'; 
				pout++; 
			}
		}
		p++; 
	}
	*outsize = (unsigned int)pout - (unsigned int)outbuf ; 
	return 1; 
}

int UrlDecode(config* pconfig, char *inbuf, int insize, char *outbuf, int *outsize)
{
	int rc = 1;
	unsigned char *p, *pe; 
	unsigned char *pout; 
	pout = outbuf; 
	p = inbuf; 
	pe = inbuf + insize -1; 
	
	while(p <= pe)
	{
		if (*p == '=')
		{
			*pout = 2; 
			pout++; 
		}
		else if (*p == '&')
		{
			*pout = 1; 
			pout++; 
		}
		else if (*p == '+')
		{
			*pout = ' '; 
			pout++; 
		}
		else if (*p == '%')
		{
			unsigned char c1, c2; 
			c1 = g_UrlDecodeTable[*(p+1)]; 
			if (*(p+1) == 'u') {
				*pout = *p; 
				pout++; 
			}
			else {
				unsigned char cm;
				c2 = g_UrlDecodeTable[*(p+2)]; 
				cm = (c1 << 4) + c2;
				
				if (pconfig->m_iNotProcessPercent25 == 1) {
					if (cm == '%') { // ע�⣺����Ϊ�˴��� '%' ��������ַ� %25 ���룬 ������Ĺ����г�ͻ��������������Ĵ��룩
						// 2010-11-01 
						*pout++ = *p++; 
						continue;
					}
				}
				
				if (cm == '%' && p + 4 <= pe) {
					p += 2;
					c1 = g_UrlDecodeTable[*(p+1)]; 
					c2 = g_UrlDecodeTable[*(p+2)]; 
					rc = 2;
				}	
				*pout = (c1 << 4) + c2; 
				pout++; 
				p+=2; 
			}
		}
		else
		{
			*pout = *p; 
			pout++; 
		}
		p++; 
	}
	*outsize = (unsigned int)pout - (unsigned int)outbuf ; 
	return rc; 
}

int DataDecode(config* pconfig, char *inbuf, int insize, char *outbuf, int *outsize)
{	
	unsigned char *p, *pe; 
	unsigned char *pout; 
	pout = outbuf; 
	p = inbuf; 
	pe = inbuf + insize -1; 
	
	while(p <= pe)
	{
		if (*p == '=')
		{
			*pout = 2; 
			pout++; 
		}
		else if (*p == '&')
		{
			*pout = 1; 
			pout++; 
		}
		else if (*p == '+')
		{
			*pout = ' '; 
			pout++; 
		}
		else if (*p == '%')
		{
			unsigned char c1, c2; 
			c1 = g_UrlDecodeTable[*(p+1)]; 
			if (*(p+1) == 'u') {
				*pout = *p; 
				pout++; 
			}
			else {
				unsigned char cm;
				c2 = g_UrlDecodeTable[*(p+2)]; 
				cm = (c1 << 4) + c2;
				
				if (pconfig->m_iNotProcessPercent25 == 1) {
					if (cm == '%') { // ע�⣺����Ϊ�˴��� '%' ��������ַ� %25 ���룬 ������Ĺ����г�ͻ��������������Ĵ��룩
						// 2011-01-06
						*pout++ = *p++; 
						continue;
					}
				}
				
				if (cm == '%' && p + 4 <= pe) {
					p += 2;
					c1 = g_UrlDecodeTable[*(p+1)]; 
					c2 = g_UrlDecodeTable[*(p+2)]; 					 
				}	
				*pout = (c1 << 4) + c2; 
				pout++; 
				p+=2; 
			}
		}
		else
		{
			*pout = *p; 
			pout++; 
		}
		p++; 
	}
	*outsize = (unsigned int)pout - (unsigned int)outbuf ; 
	return 1; 
}

char *GetPathOfAbsUrl(pool *apool, char *pabsurl)
{
	char *p; 	
	p = strstr(pabsurl, "://"); 
	if (!p)
		return NULL; 
	p+=3; 
	p = strchr(p, '/'); 
	if (!p)
		p = apr_pstrdup(apool, "/"); 
	
	return p; 
}

char * strnistr(char *s1, char *s2, int n)
{
	int i; 
	int l; 
	l = strlen(s2); 
	for(i=0; i<n; i++)
	{
		if (strnicmp(s1+i, s2, l)==0)
		{
			return s1+i; 
		}
	}
	return NULL; 
}

char *FindMatch(char *pbuf, int nbuf, int c)
{
	char *pcur, *pend; 
	ifstack lstack; 
	int liCurQuote; 
	int liOldQuote; 
	
	StackInit(&lstack); 
	pcur = pbuf; 
	pend = pbuf + nbuf - 1; 
	while (pcur<=pend)	
	{
		if ((*pcur & 0x80) != 0) 
		{
			pcur+=2; 
			continue; 
		}
		else if(*pcur == c)
		{
			if (StackEmpty(&lstack))
			{
				return pcur; 
				break; 
			}
		}
		
		if (*pcur == '\'' || *pcur=='"')
		{
			if (*pcur=='\'')
				liCurQuote = SINGLE_QUOTE; 
			else if (*pcur=='"')
				liCurQuote = DOUBLE_QUOTE; 
			
			if (StackPop(&lstack, &liOldQuote) == 0)
			{
				StackPush(&lstack, liCurQuote); 
			}
			else if (liCurQuote != liOldQuote)
			{
				StackPush(&lstack, liOldQuote);             
			}
			
			pcur++; 
			continue; 
		}
		pcur++; 
	}
	
	return NULL; 
}

char *FindMatchToken(char *pbuf, int nbuf, char *token, int ntoken)
{
	char *p; 
	char *pend; 
	char *pcur; 
	pcur = pbuf; 
	pend = pbuf + nbuf-1; 
	while(pend - pcur - ntoken > 0)
	{
		p = FindMatch(pcur, pend - pcur - ntoken, *token); 
		if (!p)
			return NULL; 
		if (strnicmp(p, token, ntoken)==0)
			return p; 
		else
			pcur = p + 1; 
	}
	return NULL; 
}

char *strnchr(char *s, int nsize, int c)
{
	char *p; 
	char *pend; 
	pend = s + nsize -1; 
	for(p = s; p<=pend && *p!=0;)
	{
		if ((*p & 0x80) != 0)
			p+=2; 
		else if (*p==c) return p; 
		else
			p++; 
	}
	return NULL; 
}

char* rsfind(char *pbeg, char *pend, char c)
{
	char *ptr = pend;
	
	while (ptr >= pbeg) {
		if ((*ptr & 0x80) != 0) {
			ptr -= 2;
		}
		else if (*ptr == c) {
			return ptr; 
		}
		else {
			ptr--; 
		}
	}
	
	return NULL; 
}

char* build_absolute_path(pool *apool, ConvertCtx *apCtx, config* pconf, char *apurl, int nsize)
{
	char *lpNewPath; 
	char *lpBaseUrl, *lpUrl; 
	char *baseurl; 
	char url[2560]; 
	char *p, *p1, *pt = apurl + nsize; 
	char *lpbasedomain;
	
//	if ((nsize * 2) > (apCtx->nBuffArraySize - 256)) {
//		return NULL;
//	}
    if(nsize > 2048){
        return NULL;
    }
	
	baseurl = apCtx->m_pcCurrentUrl;     
	p = apurl; 
	p1 = apurl + nsize; 
	if (*apurl == '\'' || *apurl == '"') {
		p = apurl + 1; 
		p1 = FindMatch(p, nsize - 1, *apurl); 
		if (!p1) {
			return NULL; 
		}
	}
	
	while((p < pt) && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
		p++; 
	}
	if (*p == '#' || p == pt) {
		return NULL; 
	}
	
	if ((p1 - p) < sizeof(url) - 1) {
		memcpy(url, p, p1 - p); 
	}
	else {
		return NULL; 
	}    
	url[p1 - p] = 0; 
	
	if (url[0]=='?') {
		lpNewPath = apr_palloc(apCtx->p,2350);
		strcpy(lpNewPath, baseurl); 
		{
			char *pask = strchr(lpNewPath, '?'); 
			if (pask) {
				*pask = '\0'; 
			}
		}
		strcat(lpNewPath, url); 
		return lpNewPath; 
	}
	
	if (strnchr(url, 16, ':')) {
		lpNewPath = lpNewPath = apr_palloc(apCtx->p,2350);
		strcpy(lpNewPath, url); 
		return lpNewPath; 
	}
	if (*url == '/' && *(url + 1) == '/' && *(url + 2) != ':') {
	    //处理入"//www.baidu.com/a.jpg"这样的url
		char *pcur = strchr(baseurl, ':');
		if (pcur != NULL) {
			lpNewPath = lpNewPath = apr_palloc(apCtx->p,2350);
			memcpy(lpNewPath, baseurl, pcur - baseurl + 1);
			strcpy(lpNewPath + (pcur - baseurl + 1), url);
			return lpNewPath;
		}	
	}

	
	p = strchr(baseurl, ':'); 
	if (!p) {
		return NULL; 
	}
	p++; 
	while(*p == '/')p++; 
	lpbasedomain = strchr(p, '/'); 
	if (!lpbasedomain)
		return NULL; 
	
	lpBaseUrl = strchr(lpbasedomain, '?'); 
	if (lpBaseUrl) {
		lpBaseUrl = rsfind(lpbasedomain, lpBaseUrl, '/'); 
	}
	else {
		lpBaseUrl = strrchr(lpbasedomain, '/'); 
	}
	
	lpUrl = url; 
	if (*lpUrl == '/') {
		lpBaseUrl = lpbasedomain; 
		lpUrl++; 
	}
	else {
		char *p; 
		while(1) {
			if (*lpUrl != '.') {
				break; 
			}
			if ((*lpUrl == '.') && (*(lpUrl+1) == '.')) {
				if (lpBaseUrl > lpbasedomain) {
					lpBaseUrl--; 
					lpBaseUrl = rsfind(lpbasedomain, lpBaseUrl, '/'); 
				}
			}
			p = strchr(lpUrl, '/'); 
			if (p == NULL) {
				break; 
			}
			lpUrl = p+1; 
		}
	}
	
	if (lpBaseUrl == NULL) {
		return NULL;
	}
	lpNewPath =  apr_palloc(apCtx->p,2350);
	memcpy(lpNewPath, baseurl, lpBaseUrl - baseurl + 1); 
	lpNewPath[lpBaseUrl - baseurl + 1] = '\0'; 
	strcat(lpNewPath, lpUrl); 	
	return lpNewPath; 
}

char *BuildBase64Block(ConvertCtx *apCtx, char *srcStr, int nsize)
{
	char buf[3072]; 
	char buf1[64]; 
	char *p, *p1; 
	int len; 
	
	if ((nsize * 2) > (apCtx->nBuffArraySize - 256)) {
		p = GetBuffArray(apCtx, 2, 0); 
		memcpy(p, srcStr, nsize); 
		p[nsize] = 0; 
		return p; 
	}
	len = if_xbase64encode(buf, srcStr, nsize) - 1; 
	p = GetBuffArray(apCtx, 2, 0); 
	p1 = p; 
	memcpy(p, "-base", 5); 
	p += 5;     
	itoa(len, buf1, 10); 
	memcpy(p, buf1, strlen(buf1));     
	p += strlen(buf1);     
	*p++ = '-'; 
	memcpy(p, buf, len); 
	p += len; 
	*p = 0; 
	return p1; 
}

char *DoUrlMap(pool *apool, ConvertCtx *apCtx, config* pconf, char *pabsurl)
{
	array_header *reqhdrs_arr; 
	table_entry *reqhdrs; 
	int i; 
	
	reqhdrs_arr = (array_header *) apr_table_elts(pconf->m_pUrlMap); 
	if (!reqhdrs_arr) {
		return 0;
	}
	reqhdrs = (table_entry *) reqhdrs_arr->elts; 
	for (i=0; i<reqhdrs_arr->nelts ; i++)
	{
		char *fromUrl, *toUrl; 
		fromUrl = reqhdrs[i].key; 
		toUrl = reqhdrs[i].val; 
		if (strnicmp(fromUrl, pabsurl, strlen(fromUrl))==0)
		{
			memmove(pabsurl+strlen(toUrl), pabsurl+strlen(fromUrl), strlen(pabsurl) - strlen(fromUrl)+1); 
			memcpy(pabsurl, toUrl, strlen(toUrl)); 
			return pabsurl; 
		}
	}
	
	return 0;     
}

char *DoUrlMapNE(pool *apool, ConvertCtx *apCtx, config* pconf,char *pabsurl)
{
	array_header *reqhdrs_arr; 
	table_entry *reqhdrs; 
	int i; 
	
	reqhdrs_arr = (array_header *) apr_table_elts(pconf->m_pUrlMapNE); 
	if (!reqhdrs_arr) {
		return 0;
	}
	reqhdrs = (table_entry *) reqhdrs_arr->elts; 
	for (i=0; i<reqhdrs_arr->nelts ; i++)
	{
		char *fromUrl, *toUrl; 
		fromUrl = reqhdrs[i].key; 
		toUrl = reqhdrs[i].val; 
		
		if (strnicmp(fromUrl, pabsurl, strlen(fromUrl)) == 0) {
			if (strlen(toUrl) < strlen(fromUrl)) {
				memmove(pabsurl + strlen(toUrl), pabsurl + strlen(fromUrl), strlen(pabsurl) - strlen(fromUrl) + 1); 
				memcpy(pabsurl, toUrl, strlen(toUrl)); 				 
				return pabsurl;
			}
			else {
				char *newurl = apr_palloc(apool, LOGALLOCSIZE(strlen(pabsurl) + strlen(toUrl))); 
				
				strcpy(newurl, toUrl);
				strcat(newurl, pabsurl + strlen(fromUrl));
				return newurl;
			}			
		}
	}
	
	return 0;     
}

char *DoUrlModify(pool *apool, ConvertCtx *apCtx, config* pconf,char *url)
{
	int i = 0;
	array_header *reqhdrs_arr; 
	table_entry *reqhdrs; 
	char *pabsurl = url;
	
	reqhdrs_arr = (array_header *) apr_table_elts(pconf->m_pUrlModify); 
	if (!reqhdrs_arr) {
		return url;
	}
	reqhdrs = (table_entry *) reqhdrs_arr->elts; 
	for (i=0; i<reqhdrs_arr->nelts ; i++)
	{
		char *fromUrl, *toUrl; 
		
		fromUrl = reqhdrs[i].key; 
		toUrl = reqhdrs[i].val; 		
		pabsurl = url_reset_prefix(pabsurl, fromUrl, toUrl, apool);
	}
	if (pabsurl != url) {
		strcpy(url, pabsurl);
	}
	
	return url;
}

char* UrlUnMap(config *pconf, pool *apool, char *pabsurl)
{
	int i = 0;
	table_entry *reqhdrs = NULL;
	array_header *reqhdrs_arr = NULL;
	
	reqhdrs_arr = (array_header *) apr_table_elts(pconf->m_pUrlMap); 
	if (reqhdrs_arr == NULL) {
		return NULL;
	}
	reqhdrs = (table_entry *) reqhdrs_arr->elts; 
	for (i = 0; i < reqhdrs_arr->nelts; i++) {
		char *toUrl = reqhdrs[i].key; 
		char *fromUrl = reqhdrs[i].val;
		
		if (strnicmp(fromUrl, pabsurl, strlen(fromUrl)) == 0) {
			if (strlen(toUrl) < strlen(fromUrl)) {
				memmove(pabsurl + strlen(toUrl), pabsurl + strlen(fromUrl), strlen(pabsurl) - strlen(fromUrl) + 1); 
				memcpy(pabsurl, toUrl, strlen(toUrl)); 				 
				return pabsurl;
			}
			else {
				char *newurl = apr_palloc(apool, LOGALLOCSIZE(strlen(pabsurl) + strlen(toUrl))); 
				
				strcpy(newurl, toUrl);
				strcat(newurl, pabsurl + strlen(fromUrl));
				return newurl;
			}			
		}
	}
	
	return NULL;
}

int IsPicture(char *url, config *pcon)
{
	if (pcon->m_iProcessPicture != 1 && pcon->m_iSeekPhoto == 0) {
		char *p = url; 
		char *ptr = url + strlen(url) - 4;
		
		while (p < ptr + 4) {
			if (*p & 0x80 || *p == '%' || *p == '?') {
				return 0; 
			}
			++p; 
		}
		
		if (!strnicmp(ptr, ".jpg", 4) || !strnicmp(ptr, ".swf", 4) || 
			!strnicmp(ptr, ".gif", 4) || !strnicmp(ptr, ".bmp", 4) || !strnicmp(ptr, ".png", 4)) {
			return 1; 
		}
		if (!strnicmp(ptr - 1, ".jpeg", 5)) {
			return 1; 
		}
	}
	
	return 0; 
}

int KeepUrlSuffix(ConvertCtx *apCtx, config *dconf, char *pUrl)
{
	svr_config *conf = apCtx->svr_conf;
	
	if (conf == NULL) {
		return 0;
	}

	if (conf->m_iKeepUrlSuffix > 0) {
		int i = 0;
		char *p = pUrl;
		char *pend = pUrl + strlen(pUrl);
		
		while (p < pend) {
			//有%和?和中文的url,都不属于keepUrlSuffix的范围
			if ((*p & 0x80) || *p == '%' || *p == '?') {
				return 0; 
			}
			++p; 
		}
		while (i < conf->m_iKeepUrlSuffix) {
			int len = strlen(conf->m_pcKeepUrlSuffix[i]);
			if (strnicmp(conf->m_pcKeepUrlSuffix[i], pend - len, len) == 0) {
				return 1;
			}
			i++;
		}
	}
	
	return 0;
}

int ChangeUrl(pool *apool, ConvertCtx *apCtx, config* pconf, char *apurl, int nsize, char **pNewUrl, int *nNewUrl)
{
	char *pabsurl; 	
	char *p, *pquery;     
	char *ptr = apurl;
	char *lpbasedomain; 	

	if(*apurl == '#' || *apurl=='?'){
        *pNewUrl = apurl;
        *nNewUrl = nsize;
        return 0;
    }
	if (nsize > 2048 || pconf->m_iShouldChangeUrlInServer == 0 || apCtx->nNotChangeTextboxUrl > 0) {
		*pNewUrl = apurl; 
		*nNewUrl = nsize; 
		return 0; 
	}	
	if (pconf->m_iIgnoreUrlPrefix == 1 &&
		(strnistr(apurl, pconf->m_pcUrlPrefix, nsize) != NULL ||
		strnistr(apurl, pconf->m_pcSUrlPrefix, nsize) != NULL)
		) {		
		*pNewUrl = apurl; 
		*nNewUrl = nsize; 
		return 1; 
	}
	
	while (ptr < apurl + nsize) {

	    //TODO:2018-4-20 zxy,好像不对呀，为啥要替换？
		if (*ptr == '\\') {
			*ptr = '/';
		}
		else if (*ptr == '?') {
			break; 
		}
		else if (*ptr == '*')  {
		    //碰到*号就不做处理又是什么原因呢？
			*pNewUrl = apurl; 
			*nNewUrl = nsize; 
			return 0; 
		}
		else if (*ptr == '{' && *(ptr + 1) == '$')  {			
			*pNewUrl = apurl; 
			*nNewUrl = nsize; 
			return 0; 
		}
		ptr++; 
	}	
	{
	    //这里等于是 rtrim
		char *pTail = apurl + nsize - 1; 
		while(*pTail == ' ') {
			pTail--; 
			nsize--; 
		}
	}
	
	pabsurl = build_absolute_path(apool, apCtx, pconf, apurl, nsize); 
	if (!pabsurl) {
		return 0;
	}
	if (strnicmp(pabsurl, "http://", 7) != 0  && strnicmp(pabsurl, "https://", 8) != 0) {
	    //如果build出来的url不是http或者https开头，证明出错了？比如是FTP呢？
		*pNewUrl = apurl;
		*nNewUrl = nsize;
		return 0;
	}
	
	{
		if (KeepUrlSuffix(apCtx, pconf, pabsurl) != 0) {
			*pNewUrl = pabsurl;
			*nNewUrl = strlen(pabsurl); 
			return 1; 
			
		}
		if (IsPicture(pabsurl, pconf) 
			|| apCtx->fjtignoreurl == 1) {
			*pNewUrl = pabsurl; 
			*nNewUrl = strlen(pabsurl); 
			return 1; 
		}
	}
	
	{
		pabsurl = DoUrlModify(apool, apCtx, pconf, pabsurl);
	}
	
	if (DoUrlMapNE(apool, apCtx, pconf, pabsurl)) {
		*pNewUrl = pabsurl;
		*nNewUrl = strlen(pabsurl);
		return 1;
	}
	
	pabsurl = ChangeChinese(apool, apCtx, pconf, pabsurl, strlen(pabsurl));
	
	p = strchr(pabsurl, ':'); 
	if (!p) {
		return 0;
	}	
	p += 3;
	lpbasedomain = p;
	
	pquery = strchr(lpbasedomain, '?'); 
	p = strchr(lpbasedomain, '/'); 
	if ((!p) || (pquery != NULL && p > pquery)) {
		if (pquery != NULL && p > pquery) {
			memmove(pquery + 1, pquery, strlen(pquery)); 
			*pquery = '/'; 
		}
		else {
			int nlen = strlen(lpbasedomain);
			if ((pconf->m_iAddTrailingSlash != 0) 
				&& (nlen > 0)
				&& (*(lpbasedomain + nlen - 1) != ':')
				&& (*(lpbasedomain + nlen - 1) != '-')
				) {
				strcat(lpbasedomain, "/"); 
			}
		}
	}
	
	if (pconf->m_pcNativeDomain) {
		char *p = NULL;
		if (strnicmp(pconf->m_pcNativeDomain, lpbasedomain, strlen(pconf->m_pcNativeDomain)) == 0) {
			p = strchr(lpbasedomain, '/'); 
			*pNewUrl = p; 
			*nNewUrl = strlen(p); 
			return 1; 
		}
	}
	
	if (DoUrlMap(apool, apCtx, pconf, pabsurl)) {
		*pNewUrl = pabsurl; 
		*nNewUrl = strlen(pabsurl); 
		return 1; 
	}
	
	if (pconf->m_iAddUrlPrefixToParameter == 1) {
		pabsurl = url_reset_prefix(pabsurl, "http://", pconf->m_pcUrlPrefix, apool);
		pabsurl = url_reset_prefix(pabsurl, "https://", pconf->m_pcSUrlPrefix, apool);		
	}
	else {	
		if (strnicmp(pabsurl, "http:", 5) == 0) {
			p = apr_palloc(apool,strlen(lpbasedomain) + strlen(pconf->m_pcUrlPrefix) + 2);
			strcpy(p,pconf->m_pcUrlPrefix);
			strcat(p,lpbasedomain);
			pabsurl = p;
		}
		else {
			/*p = lpbasedomain - strlen(pconf->m_pcSUrlPrefix);
			memcpy(p, pconf->m_pcSUrlPrefix, strlen(pconf->m_pcSUrlPrefix));
			pabsurl = p;*/

			p = apr_palloc(apool,strlen(lpbasedomain) + strlen(pconf->m_pcSUrlPrefix) + 2);
			strcpy(p,pconf->m_pcSUrlPrefix);
			strcat(p,lpbasedomain);
			pabsurl = p;
		}
	}
	
	pabsurl = ConvertUrl(apCtx, apool, pconf, pabsurl);
	*pNewUrl = pabsurl;
	*nNewUrl = strlen(pabsurl);
	
	return 1; 
}

char* ChangeChinese(pool *apool, ConvertCtx *apCtx, config *pconf, char *apurl, int nsize)
{
	int n, len; 
	char *pb, *pe, *pt, *pend, *pnurl; 
	char temp[2560]; 
	char *pin = apurl, *pout = temp; 
	
	if ((nsize * 2) > (apCtx->nBuffArraySize - 256) || pconf->m_iChangeChineseLevel == 0) {
		return apurl; 
	}
	
	{
		n = nsize; 
		while (pin < apurl + nsize) {
			//找到回车换行
			if (*pin != '\r' && *pin != '\n') {
				*pout++ = *pin;                
			}
			else {
				--n; 
			}
			++pin; 
		}

		if (n != nsize) {
			*pout = '\0'; 
			strcpy(apurl, temp); 
			nsize = n; 
		}
	}
	
	pend = apurl + nsize; 
	pt = apurl; 
	while (pt < pend) {
		if (*pt == ':') {
			pt += 3; 
		}
		else if (*pt == '/') {
			//找到第一个 "/"
			break; 
		}
		++pt; 
	}
	if (pt >= pend) {
		return apurl; 
	}
	pb = ++pt; 
	
	while (pt < pend) {
		//有汉字，有%, 有' 有"
		if ((*pt & 0x80) || *pt == '%' || *pt == '\'' || *pt == '\"') {
			break; 
		} 
		++pt; 
	}

	//如果有问号等特殊符号,则需要base64
	if (pt >= pend) {
		return apurl; 
	}
	else if (pconf->m_iChangeChineseLevel > 0) {
		pb = pt; 
	}
	
	pnurl = GetBuffArray(apCtx, 3, apCtx->prefix_len); 
	memcpy(pnurl, apurl, pb - apurl); 
	pout = pnurl + (pb - apurl); 
	
	if (pconf->m_iChangeChineseLevel == 1) {
		memcpy(pout, "-ifbase1", 8); 
		pout += 8; 
		len = if_xbase64encode(pout, pb, pend - pb); 
		pout += len; 
		*pout = '\0'; 
		return pnurl; 
	}
	if (pconf->m_iChangeChineseLevel == 3) {
		memcpy(pout, "-ifbase3", 8); 
		pout += 8; 
	}
	if (pconf->m_iChangeChineseLevel == 4) {		
		memcpy(pout, "-ifbase4", 8); 
		pout += 8; 
	}
	
	pe = pb + 1; 
	while (pb < pend) {
		while (*pe != '/' && *pe != ';' && *pe != '?' && pe < pend) {
			++pe; 
		}
		
		{
			char blen[64]; 
			char buff[2560]; 
			char *ptest = pb; // WPF 2008-07-16
			
			while (ptest < pe) {
				if (*ptest == '%' || (*ptest & 0x80)) {
					ptest = NULL;
					break;
				}
				++ptest;
			}
			if (ptest == NULL) {
			    //如果有%，表明有中文
				if (pconf->m_iChangeChineseLevel < 5) {
					len = if_xbase64encode(buff, pb, pe - pb) - 1; 
					if (len > 0) {                
						memcpy(pout, "-base", 5); 
						pout += 5; 
						itoa(len, blen, 10); 
						n = strlen(blen); 
						memcpy(pout, blen, n); 
						pout += n; 
						*pout++ = '-'; 
						memcpy(pout, buff, len); 
						pout += len; 
					}
				}
				else {
				    //如果大于5表明要对url进行转码，回来的时候再反转码
					int nin = pe - pb;
					
					if (pout - pnurl + nin * 4 >= apCtx->nBuffArraySize) {
						return apurl;
					}
					len = ConvertToUnicodeExt(pconf->m_iFromEncode, FFFE, 1, pb, nin, pout);					
					if (len > 0) {
						pout += len;
					}
				}
			}
			else {
				//如果根本没有中文，则直接输出
				memcpy(pout, pb, pe - pb);
				pout += pe - pb;
			}
		}
		
		if (pe < pend) {
			*pout++ = *pe; 
			if (*pe == '?' && pconf->m_iChangeChineseLevel < 4) {
				pb = pe + 1; 
				pe = pend - 1; 
				continue; 
			}            
		}
		
		pb = ++pe; 
	}
	
	*pout = '\0'; 
	return pnurl; 
}

int UrlEncodeHZ(char *inbuf, int insize, char *outbuf)
{// WPF This function is not summit to guarantee
	unsigned char *pi = inbuf, *po = outbuf; 
	unsigned char *pend = inbuf + insize; 
	unsigned char ch, cl; 
	
	while (pi < pend) {
		if (*pi & 0x80) {
			ch = *pi >> 4; 
			cl = *pi & 0x0F; 
			if (ch > 9) {
				ch += 'A' - 10; 
			}
			else {
				ch += '0'; 
			}
			if (cl > 9) {
				cl += 'A' - 10; 
			}
			else {
				cl += '0'; 
			}
			*po++ = '%'; 
			*po++ = ch; 
			*po++ = cl;            
		}
		else {
			*po++ = *pi;            
		}
		++pi; 
	}
	
	return (char *)po - outbuf; 
}

char* UnChangeChinese(pool *apool, config *pconfig, ConvertCtx *apCtx, char *apurl)
{
	int i, len, liChangeChineseLevel; 
	char *pathb, *p, *pend, *pnewurl; 
	char lcxUrl[3072]; 
	
	len = strlen(apurl); 
	pend = apurl + len; 
	
	if (len > 2048 || pconfig->m_iChangeChineseLevel == 0) {
		return apurl; 
	}
	
	if (pathb = strnistr(apurl, "-ifbase1", len)) {
		liChangeChineseLevel = 1; 
	}
	else if (pathb = strnistr(apurl, "-ifbase3", len)) {
		liChangeChineseLevel = 3; 
	}
	else if (pathb = strnistr(apurl, "-ifbase4", len)) {
		liChangeChineseLevel = 4; 
	}
	
	if (!pathb) {
		liChangeChineseLevel = 4; 
	}
	
	pnewurl = apr_palloc(apool, LOGALLOCSIZE(len + 64)); 
	p = lcxUrl;
	
	if (pathb) {
		memcpy(p, apurl, pathb - apurl); 
		p += pathb - apurl; 
		pathb += 8; 
	}
	else {
		pathb = strnistr(apurl, "://", len);
		if (pathb != NULL) {
			pathb = strchr(pathb + 3, '/');	
			if (pathb != NULL) {
				memcpy(p, apurl, pathb - apurl); 
				p += pathb - apurl; 				
			}
			else {
				pathb = apurl;
			}
		}
		else {
			pathb = apurl;
		}
	}
	
	if (liChangeChineseLevel < 2) {
		len = if_xbase64decode(p, pathb); 
		p += len; 
		*p++ = '\0'; 
	}
	else {
		char *pb, *pe;        
		pb = pathb; 
		
		while(pb < pend) {
			pe = strnistr(pb, "-base", pend - pb); 
			
			if (!pe) {
				memcpy(p, pb, pend - pb); 
				p += pend - pb; 
				*p++ = '\0'; 
				break; 
			}
			else {                
				int i, len; 
				char buf[2560]; 
				
				i = 0; 
				if (pe > pb) {
					memcpy(p, pb, pe - pb); 
					p += pe - pb; 
				}
				pb = pe;
				pe += 5; 				
				while (*pe != '-' && pe < pend) {                    
					buf[i++] = *pe++; 
				}
				if (pe < pend) {
					++pe; 
					buf[i] = '\0'; 
					len = atoi(buf);                    
					if (len < 0 || pe + len > pend) {
						return apurl; 
					}                
					memcpy(buf, pe, len);                
					buf[len] = '\0'; 
					pe += len; 
					len = if_xbase64decode(p, buf); 
					p +=  len; 
				}
				else {
					*p++ = *pb++;
					continue;
				}
				pb = pe;                
			}
		}
		*p++ = '\0';
	} 
	
	p = pnewurl; 
	len = strlen(lcxUrl); 
	for (i = 0; i < len; i++) {
		if (lcxUrl[i] == ' ') {
			*p++ = '%'; 
			*p++ = '2'; 
			*p++ = '0'; 
		}
		else if (lcxUrl[i] == '\r' || lcxUrl[i] == '\n') {            
			NULL; 
		}
		else if (strnicmp(&lcxUrl[i], "&amp; ", 5) == 0) {
			*p++ = '&'; 
			i += 4; 
		}
		else {
			*p++ = lcxUrl[i]; 
		}
	}    
	*p++ = '\0';  
	
	{// WPF 
		if ((pconfig->m_iForcePostUTF8 == 1) && (apCtx == NULL || apCtx->nUrlEncode != ENCODE_UTF8)) { int nt, np = strlen(pnewurl);
			char utf[2560];            
			nt = ConvertToUTF8Ex(pconfig->m_iFromEncode, pnewurl, np, utf, pconfig); 
			if (nt != np) {
				char *pencodeurl = apr_palloc(apool, LOGALLOCSIZE(nt * 3 + 128)); 
				nt = UrlEncodeHZ(utf, nt, pencodeurl); 
				*(pencodeurl + nt) = '\0'; 
				{// convert &#12345; to UTF-8 [URL]
					int n = url_unicode2utf8(pencodeurl, utf);
					if (n > 0) {
						pencodeurl = apr_pstrdup(apool, utf);
					}
				}
				return pencodeurl; 
			}
		}
		else if ((pconfig->m_iSendURLsAsUTF8 == 1) && (apCtx == NULL || apCtx->nUrlEncode != ENCODE_UTF8)) {
			int nt, np = strlen(pnewurl); 
			char utf[2560];
			char *pq = strchr(pnewurl, '?');
			if (pq == NULL){
				nt = ConvertToUTF8Ex(pconfig->m_iFromEncode, pnewurl, np, utf, pconfig); 
				if (nt != np) {
					char *pencodeurl = apr_palloc(apool, LOGALLOCSIZE(nt * 3 + 128)); 
					nt = UrlEncodeHZ(utf, nt, pencodeurl); 
					*(pencodeurl + nt) = '\0'; 
					{// convert &#12345; to UTF-8 [URL]
						int n = url_unicode2utf8(pencodeurl, utf);
						if (n > 0) {
							pencodeurl = apr_pstrdup(apool, utf);
						}
					}
					return pencodeurl; 			
				}
			}
			else {
				int nf = pq - pnewurl;
				nt = ConvertToUTF8Ex(pconfig->m_iFromEncode, pnewurl, nf, utf, pconfig); 
				if (nt != nf) {
					char *pencodeurl = apr_palloc(apool, LOGALLOCSIZE((nt + (np - nf)) * 3 + 128)); 
					nt = UrlEncodeHZ(utf, nt, pencodeurl); 
					*(pencodeurl + nt) = '\0'; 					
					{// convert &#12345; to UTF-8 [URL]
						int n = url_unicode2utf8(pq, utf);						
						if (n > 0) {
							strcpy(pencodeurl + nt, utf);							
						}
						else {
							strcpy(pencodeurl + nt, pq);							
						}
					}
					return pencodeurl; 
				}
			}
		}
	}
	
	return pnewurl; 
}

// this is a replace function, replace all s => d in u
char* url_reset_prefix(char *u, char *s, char *d, pool *p)
{
	if (*s == '\0' || *d == '\0') {
		return u;
	}
	else {
		char *h = u;
		char *ptr = strstr(h, s);
		memstream *ms = NULL;
		
		if (ptr == NULL) {
			return u;
		}
		ms = create_memstream(p);		
		while (ptr != NULL) {
			memstream_write(ms, h, ptr - h);
			h = ptr + strlen(s);
			memstream_write(ms, d, strlen(d));
			ptr = strstr(h, s);
			if (ptr == NULL) {
				int n = 0;
				
				memstream_write(ms, h, strlen(h));	
				memstream_write(ms, "\0", 1);
				n = get_memstream_datasize(ms);
				ptr = apr_palloc(p, n);
				memstream_read(ms, ptr, n);			
				return ptr;				
			}
		}
	}
	
	return u;
}

// 2007.8.28 www.56.com 

char* url_http_proxy(char *url, char *http, char *proxy, pool *p)
{
	char *ptr = strstr(url, "http");
	
	if (ptr == NULL) {
		return url;
	}
	else {
		int n = 0;
		int nu = strlen(url);
		int nh = strlen(http);
		int np = strlen(proxy);
		char *b = url;
		memstream *ms = create_memstream(p);
		
		ptr += 4;
		ptr = strstr(ptr, http);
		while (ptr != NULL) {
			if (strnicmp(ptr, proxy, np) != 0) {
				memstream_write(ms, b, ptr - b);			
				memstream_write(ms, proxy, np);
				b = ptr + nh;
			}
			ptr += nh;				
			ptr = strstr(ptr, http);			
		}
		if (b == url) {
			return url;
		}
		memstream_write(ms, b, url + nu - b);
		memstream_write(ms, "\0", 1);
		n = get_memstream_datasize(ms);
		if (n > nu) {
			ptr = apr_palloc(p, n + 1);
			memstream_read(ms, ptr, n);			
			return ptr;
		}
	}
	
	return url;
}

// WPF //
apr_port_t get_addr_domain_host_port(hosts_addr_domain *addrdomain, const char *hostname, apr_port_t port)
{
	hosts_addr_domain *match = addrdomain;
	
	while (match != NULL) {		
		char *domain = match->domain;
		
		if (*domain == '*') {
			if (match->domain_port == 0 || match->domain_port == port) {
				break;
			}
		}
		else if (*domain == '.') {
			int nd = strlen(domain);
			int nh = strlen(hostname);
			
			if (nh >= nd) {
				if (strncasecmp(hostname + (nh - nd), domain, nd) == 0) {
					if (match->domain_port == 0 || match->domain_port == port) {
						break;
					}
				}
			}
		}
		else {
			int nd = strlen(domain);
			int nh = strlen(hostname);
			
			if (nd == nh && strncasecmp(hostname, domain, nd) == 0) {
				if (match->domain_port == 0 || match->domain_port == port) {
					break;
				}
			}
		}
		match = match ->next;
	}
	
	if (match != NULL && match->ipstr_port != 0 && match->ipstr_port != port) {
		return match->ipstr_port;		
	}
	
	return 0;
}

char* trimstr(char *str)
{
	char *ptr = str + strlen(str) - 1;
	
	while ((*ptr == ' ')
		&& (ptr >= str)) {
		--ptr;
	}
	*(ptr + 1) = '\0';
	
	ptr = str;
	while (*ptr == ' ') {
		ptr++;
	}
	
	if (ptr != str) {
		memmove(str, ptr, strlen(ptr) + 1);
	}
	
	return str;
}

#ifndef WIN32
void itoa(int num,char *str,int base)
{
    int  i,j;
    char use[50];
    
    if (num == 0)
    {
        strcpy(str, "0");
        return;
    }
    
    for(i=0;num>0;i++)
    {
        use[i]=num%base+48;
        num/=base;
    }
    *(str+i)='\0';
    for(j=0;i>0;j++,i--)
        *(str+j)=use[i-1];
}
#endif

char *getDomain(char *pabsurl, apr_pool_t *pool){
    char *presult = apr_palloc(pool,256);
    char *pdomain = strnistr(pabsurl,"://",strlen(pabsurl));
    pdomain = pdomain + 3;
    char *pend = strchr(pdomain,'/');
    if(pend){
        if(pend - pdomain >255){
            return NULL;
        }
        memcpy(presult,pdomain,pend-pdomain);
        presult[pend-pdomain] = 0;
        return presult;
    }
    else{
        if(strlen(pdomain)>255){
            return NULL;
        }
        strcpy(presult,pdomain);
    }
}

// WPF //


