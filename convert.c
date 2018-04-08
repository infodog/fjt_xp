#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#ifndef  WIN32 
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#endif 

#include "config.h"
#include "convert.h"
#include "exportapi.h"
#include "baseutil.h"
#include "update.h"

/*-----------------------------------------------------------------------------------------------------------------------*/

#define  LOG

//����ṹ��
typedef struct tagINPUTINFO
{
	unsigned char *gb2big5;
	unsigned char *big52gb;
	ruletable *wordgbbig;
	ruletable *wordbiggb;
	char *pU2GB, *pU2BIG;
	char *pGB2U, *pBIG2U;
	
	unsigned char *unigbbig;
	unsigned char *unibiggb;
	ruletable *gbrule;
	ruletable *bigrule;
}INPUTINFO, *LPINPUTINFO;

/*-----------------------------------------------------------------------------------------------------------------------*/
/* Default word tables */
ruletable *wordgbbig[FJTMAXWORD],*wordbiggb[FJTMAXWORD];
unsigned  char gb2big5[256][256][2],big52gb[256][256][2];

hkword	  *hktogb[256][256];

/* Unicode format is &#12345; */
unsigned  char untogb[FJTMAXWORD][2];
unsigned  char untobig[FJTMAXWORD][2];

/* Unicode format is two bytes */
char *unicode2gb, *unicode2big, *gb2unicode, *big2unicode;

/* Get copyright */
char heaventest[128], lastdefense[128], fjtcopyright[128], gotourl[256], thetitle[1024], onlineurl[1024]; 

/* We should initial table once if init_count > 1 then not init table again */
int init_count = 0;

//Only for unicode to unicode convert
unsigned char *UniGbUniBig = NULL, *UniBigUniGb = NULL;
ruletable *UniGbUniBigRule, *UniBigUniGbRule;
/*-----------------------------------------------------------------------------------------------------------------------*/

char dir[512];
void getdir(char *str,int size)
{
#ifdef WIN32
	if (GetModuleFileName(NULL, str, size) == 0) {
		*str = '\0';
	}
	else { 
		char *ptr = NULL;
		char *pbe = strchr(str, '\\');
		
		ptr = strrchr(str, '\\');
		if (ptr != NULL && ptr != pbe) 
			*ptr = '\0'; 
		ptr = strrchr(str, '\\');
		if (ptr != NULL) 
			*(ptr + 1) = '\0';
	}
#else
	{
		strcpy(str, ap_server_root);
		if (*(str + strlen(str) - 1) != '/') {
			strcat(str, "/");
		}
	}
#endif
}



/*-----------------------------------------------------------------------------------------------------------------------*/

void myencrypt(char *str, char *out, int enc)
{
	int i, len;
	
	if (!str)
	{
		out = NULL;
		return;
	}
	
	len = strlen(str);
	
	if (enc)
	{
		out[0] = str[0] + 31;
		out[0] ^= 85;				
		for(i = 1; i < len; i++)
			out[i] = str[i] ^ out[i-1];
	}
	else
	{
		for(i = len - 1; i > 0; i--)
			out[i] = str[i] ^ str[i-1];		
		out[0] = str[0] ^ 85;
		out[0] -= 31;
	}
	
	out[len] = '\0';
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int EncodeAscii(char *pin, int size, char *pout, int flag)
{
	int i, n, nlen;	
	unsigned char ch, *pl, *pr;
	unsigned char hzc, *pd, *pt, *pend;
	
	pd = (unsigned char*)pin;
	pt = (unsigned char*)pout;
	pend = pd + size;
	
	if (flag == 0) {
		hzc = 0;
		while (pd < pend) {
			if ((*pd == '\r' || *pd == '\n') && !hzc) 
				*pt++ = *pd;
			else {
				*pt++ = *pd / 16 + 'A';
				*pt++ = *pd % 16 + 'A';
			}
			if ((*pd++ & 0x80) && !hzc) 
				hzc = 1;
			else
				hzc = 0;
		}
		nlen = (char*)pt - pout;	
		for (i = 0; i < nlen; i++) {
			n = (((i * i) % nlen) * i % nlen) * i % nlen;
			pl = (unsigned char*)pout + i;
			pr = (unsigned char*)pout + n;
			if (*pl == '\r' || *pl == '\n' || *pl == 0x0
				|| *pr == '\r' || *pr == '\n' || *pr == 0x0)
				continue;
			ch = *pl;
			*pl = *pr;
			*pr = ch;			
		}
	}		
	else {
		nlen = size;
		for (i = nlen - 1; i >= 0; i--) {
			n = (((i * i) % nlen) * i % nlen) * i % nlen;
			pl = (unsigned char*)pin + i;
			pr = (unsigned char*)pin + n;
			if (*pl == '\r' || *pl == '\n' || *pl == 0x0
				|| *pr == '\r' || *pr == '\n' || *pr == 0x0)
				continue;
			ch = *pl;
			*pl = *pr;
			*pr = ch;			
		}
		while (pd < pend) {
			ch = *pd++;
			if (ch != '\r' && ch != '\n')
				ch = (ch - 'A') * 16 + *pd++ - 'A';							
			*pt++ = ch;
		}
		nlen = (char*)pt - pout;	
	}
	
	return nlen;
}

void forcopyright(void)
{
	{
		char str[] = "GEHIGFGFHGGFGOHBHEGD";
		int len = strlen(str);
		len = EncodeAscii(str, len, heaventest, 1);
		heaventest[len] = '\0';
	}
	{
		char str[] = "GDGMGFHBHEGEGFGDHGGO";
		int len = strlen(str);
		len = EncodeAscii(str, len, lastdefense, 1);
		lastdefense[len] = '\0';
	}
	{
		char str[] = "EEEPEGEEFIEDEAFKFJFJFCEH";
		int len = strlen(str);
		len = EncodeAscii(str, len, fjtcopyright, 1);
		fjtcopyright[len] = '\0';
	}
	{
		char str[] = "GPHOGKDOGDHEHIGPCPCEHACHHOCHHGGHGJGOGDGDHPGOCFGAHBCNGPGD";
		int len = strlen(str);
		len = EncodeAscii(str, len, gotourl, 1);
		gotourl[len] = '\0';
	}
	{
		char str[] = "COHAGCGIGAGFGDHACEGCHAHGGJHJGEGACPHCGBCAHECECEHDGPGDGPHFH"\
			"AHJHEHFGAGPGPGOCIHAHHGJHEGDHGGJHACFGAGBGDGDGIGGHHGOGPGFCMGM";
		int len = strlen(str);
		len = EncodeAscii(str, len, thetitle, 1);
		thetitle[len] = '\0';
	}
	{
		char str[] = "FNEPEFEOEJEOEFFMDFFHECFBECEEEGEKEEFAFFEM";
		int len = strlen(str);
		len = EncodeAscii(str, len, onlineurl, 1);
		onlineurl[len] = '\0';
	}
}

void CheckLeftBuffer(ConvertCtx *pctx, config *pconf, char **inbuffer, int *insize)
{
	int no = 0;
	int nsize = 0;
	char *inbuf = NULL;
	
	no = pctx->bakno;
	if (no < 1) {
		return;
	}	
	nsize = *insize + no;
	
	{
		int au = 0;
		char *pau = NULL;
		
		if (pctx->n_BakBuf < nsize) {
			if (pctx->n_BakBuf > pctx->nbakinfo) {
				au = pctx->n_BakBuf;
				pau = pctx->p_BakBuf; 
			} 
			if (nsize < 8192) {
				pctx->n_BakBuf = 8192;
			}
			else {
				pctx->n_BakBuf = nsize;
			}
			pctx->p_BakBuf = apr_palloc(pctx->p, LOGALLOCSIZE(pctx->n_BakBuf)); 
		}	
		inbuf = pctx->p_BakBuf;
		memcpy(inbuf, pctx->bakinfo, no);
		memcpy(inbuf + no, *inbuffer, *insize);
		pctx->bakno=0;
		if (au > pctx->nbakinfo) {
			pctx->nbakinfo = au;
			pctx->bakinfo = pau; 
		} 
	}
	
	*insize = nsize;
	*inbuffer = inbuf;	
}

char* ismine(request_rec *r, char *url, pool *p, int *flag)
{
	char *myurl = NULL;
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ismine001) "check ismine, url=%s,fjtcopyright=%s,gotourl=%s",url,fjtcopyright,gotourl);
	*flag = 0;	
	if (// strnistr(url, heaventest, len)
		// || strnistr(url, lastdefense, len)
		strstr(url, fjtcopyright)) {
		myurl = apr_pstrdup(p, gotourl);
		if (strstr(url, "SHOWLICENSE")) {
			FILE *fp = NULL;
			int cbLicense = 0;
			char buf[1024];			
			char lcxLicense[8192];
			
			memset(buf, 0, sizeof(buf));
			memset(lcxLicense, 0, sizeof(lcxLicense));
#ifndef WIN32
			strcpy(buf, ap_server_root);
			if (buf[strlen(buf) - 1] != '/') {
				strcat(buf, "/");
			}
			strcat(buf, "etc/fjtlicense.lic");			
			fp = fopen(buf, "rb");
#else 
			GetModuleFileName(NULL, buf, sizeof(buf)); 
			{
				char *ptr = NULL;
				char *pbe = strchr(buf, '\\');        
				ptr = strrchr(buf, '\\');
				if (ptr != NULL && ptr != pbe) 
					*ptr = '\0'; 
				ptr = strrchr(buf, '\\');
				if (ptr != NULL) 
					*(ptr + 1) = '\0';
			} 
			strcat(buf, "etc\\fjtlicense.lic");
			fp = fopen(buf, "rb");
#endif   
			if (!fp) {
				return myurl;
			}    
			cbLicense = fread(lcxLicense, 1, sizeof(lcxLicense) - 1, fp);
			fclose(fp);    
			if (cbLicense == 0) {
				return myurl;
			}
			lcxLicense[cbLicense] = '\0';
			ap_rvputs(r, lcxLicense, NULL);			
			*flag = 1;
			return myurl;
		}
#ifdef WIN32
		{
			char *pn = NULL; 
			
			pn = strstr(url, onlineurl);
			if (pn != NULL) {
				static char svc[10] = {'\0'}; 				
				if (svc[0] == '\0') { 
					svc[0] = 'F', svc[1] = 'J';
					svc[2] = 'T', svc[3] = '2';
					svc[4] = '\0'; 
				} 
				pn += 20; 
				if (UpdateFJT(svc, pn) == 1) { 
					NULL;           
				} 
			}			
		}
#endif
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ismine002) "check ismine, return NOT NULL, myurl=%s",myurl);
		return myurl;
	}
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ismine003) "check ismine, return NULL");
	return NULL;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int InitConverttable()
{
	FILE *fp;
	char buf[256];
	unsigned char c1,c2,c3,c4;	
	
	buf[0] = '\0';
	
	memset(gb2big5,255,sizeof(gb2big5));
	memset(big52gb,255,sizeof(gb2big5));	
#ifdef WIN32	
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\gb2big5v2.cvt");	
	fp = fopen(dir, "rb");
#else
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/gb2big5v2.cvt");
	fp = fopen(buf, "rb");
#endif
	if (!fp) return 0;	
	while (!feof(fp)) {
		if (!fread(&c1,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n");break;}
		if (!fread(&c2,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); fclose(fp);return 0;}
		if (!fread(&c3,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); fclose(fp);return 0;}
		if (!fread(&c4,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); fclose(fp);return 0;}
#ifdef ENCRYPT_WORDS
		/* WPF 2002-8-9 Exchange c1 and c3 to decode */
		gb2big5[c3][c2][0]=c1;
		gb2big5[c3][c2][1]=c4;
#else
		gb2big5[c1][c2][0]=c3;
		gb2big5[c1][c2][1]=c4;
#endif		
	}  
	fclose(fp);
	
#ifdef WIN32
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\big52gbv2.cvt");
	fp = fopen(dir, "rb");
#else
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/big52gbv2.cvt");
	fp = fopen(buf, "rb");
#endif	
	if (!fp) return 0;
	while (!feof(fp)) {
		if (!fread(&c1,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); break;}
		if (!fread(&c2,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); fclose(fp);return 0;}
		if (!fread(&c3,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); fclose(fp);return 0;}
		if (!fread(&c4,1,1,fp)) { LOG("ERROR in loading the convert ruletable!\n"); fclose(fp);return 0;}
#ifdef ENCRYPT_WORDS		
		/* WPF 2002-8-9 Exchange c1 and c3 to decode */
		big52gb[c3][c2][0]=c1;
		big52gb[c3][c2][1]=c4;
#else
		big52gb[c1][c2][0]=c3;
		big52gb[c1][c2][1]=c4;
#endif		
	}
	fclose(fp);	
	
	return 1;
}

int initfile(ruletable *wordlist[], ruletable *reverselist[], char *filename, pool *p)
{
	FILE* fp;
	int	  flag,word,lenreal,lenrep;
	char  *ptr,line[RULELEN*2+32],realcode[RULELEN],repcode[RULELEN];
	rule  *element;
	
	if ((fp=fopen(filename,"r"))==NULL) return(0);
	while (!feof(fp)) {
#ifdef ENCRYPT_WORDS		
		{/* WPF 2002-8-9 Decode line */
			int n;
			char temp[RULELEN * 4 + 32];
			if (!fgets(temp, sizeof(temp), fp)) break;
			{/* Delete '\r' and '\n' of temp */
				int nln = strlen(temp);
				if (temp[nln - 2] == '\r')
					temp[nln - 2] = '\0';
				else if (temp[nln - 1] == '\n')
					temp[nln - 1] = '\0';
				else
					temp[nln] = '\0';
			}
			n = strlen(temp);
			n = EncodeAscii(temp, n, line, 1);
			line[n] = '\0';
		}
#else
		if (!fgets(line,sizeof(line),fp)) break;
		{/* Delete '\r' and '\n' of line */
			int nln = strlen(line);
			if (line[nln - 2] == '\r')
				line[nln - 2] = '\0';
			else if (line[nln - 1] == '\n')
				line[nln - 1] = '\0';
			else
				line[nln] = '\0';
		}		
#endif
		flag=0;
		/* Not thinking '#' symbol
		if (strchr(line,'#'))
		flag=1;
		*/
		if (!(ptr = strchr(line, '\t')) || ptr == line) continue;
		*ptr = '\0';		
		lenreal = strlen(line);
		if (!lenreal || (lenreal > RULELEN))
			continue;
		strcpy(realcode, line);
		lenrep = strlen(ptr + 1);
		if (!lenrep || (lenrep > RULELEN))
			continue;
		strcpy(repcode, ptr+1);
		
		word=realcode[0];
		if ((word & 0x80)!=0) {
			unsigned char chi = realcode[0];
			unsigned char chw = realcode[1];
			word = chi * 256 + chw;
		}
		
		element=(rule*)apr_palloc(p, sizeof(rule));
		element->lenreal=lenreal;
		element->lenrep=lenrep;
		strcpy(element->realcode,realcode);
		strcpy(element->repcode,repcode);
		element->flag=flag;
		if (wordlist[word]==NULL) {
			wordlist[word]=(ruletable*)apr_palloc(p, sizeof(ruletable));
			element->link=NULL;	
			wordlist[word]->body=element;
			wordlist[word]->flag=flag;
		}
		else {
			element->link=wordlist[word]->body;
			wordlist[word]->body=element;
			if (flag && !(wordlist[word]->flag))
				wordlist[word]->flag=flag;
		}
		
		if (reverselist != NULL) {
			word=repcode[0];
			if ((word & 0x80)!=0) {
				unsigned char chi = repcode[0];
				unsigned char chw = repcode[1];
				word = chi * 256 + chw;
			}
			
			element=(rule*)apr_palloc(p, sizeof(rule));
			element->lenreal=lenrep;
			element->lenrep=lenreal;
			strcpy(element->realcode,repcode);
			strcpy(element->repcode,realcode);
			element->flag=flag;			
			if (reverselist[word]==NULL) {
				reverselist[word]=(ruletable*)apr_palloc(p, sizeof(ruletable));
				element->link=NULL;	
				reverselist[word]->body=element;
				reverselist[word]->flag=flag;
			}
			else {
				element->link=reverselist[word]->body;
				reverselist[word]->body=element;
				if (flag && !(reverselist[word]->flag))
					reverselist[word]->flag=flag;
			}			
		}		
	} 
	
	fclose(fp); 	
	return(1);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int initunicode(void)
{
	FILE *fpgb,*fpbig;
	
	char *p;
	char gbpre[16],gbsuc[16];
	char unicode[16];
	char line[512];
	char buf[512];
	
	buf[0] = '\0';
	
	gbpre[3]=gbsuc[3]=unicode[5]='\0';
	
#ifndef WIN32	
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/unicodetogb.txt");
	if ((fpgb=fopen(buf, "rb"))==NULL)
		return 0;
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/unicodetobig.txt");
	if ((fpbig=fopen(buf, "rb"))==NULL)
		return 0;
#else
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\unicodetogb.txt");
	if ((fpgb=fopen(dir,"rb"))==NULL)
		return 0;
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\unicodetobig.txt");
	if ((fpbig=fopen(dir,"rb"))==NULL)
		return 0;
#endif
	
	memset(untogb,255,sizeof(untogb));
	memset(untobig,255,sizeof(untobig));
	
	while(!feof(fpgb))
	{
		fgets(line,sizeof(line),fpgb);
		p=strchr(line,'\t');
		memcpy(unicode,line,p-line);
		unicode[p-line]='\0';
		memcpy(gbpre,p+1,3);
		memcpy(gbsuc,p+4,3);
		untogb[atoi(unicode)][0]=(unsigned char)atoi(gbpre);
		untogb[atoi(unicode)][1]=(unsigned char)atoi(gbsuc);		
	}
	
	while(!feof(fpbig))
	{
		fgets(line,sizeof(line),fpbig);
		p=strchr(line,'\t');
		memcpy(unicode,line,p-line);
		unicode[p-line]='\0';
		memcpy(gbpre,p+1,3);
		memcpy(gbsuc,p+4,3);
		untobig[atoi(unicode)][0]=(unsigned char)atoi(gbpre);
		untobig[atoi(unicode)][1]=(unsigned char)atoi(gbsuc);		
	} 
	
	fclose(fpgb);
	fclose(fpbig);
	
	return 1; 
}

int hex2int(char *pci, int num)
{
	int hex = 0;
	char *ptr = pci, *pend = pci + num;
	
	while (ptr < pend) {
		hex <<= 4;
		if (*ptr >= '0' && *ptr <= '9') {
			hex += *ptr++ - '0';
		}
		else if (*ptr >= 'a' && *ptr <= 'f') {
			hex += *ptr++ - 'a' + 10;
		}
		else if (*ptr >= 'A' && *ptr <= 'F') {
			hex += *ptr++ - 'A' + 10;
		}
		else {
			break;
		}
	}
	
	return hex;
}

int hz_unicode(char *pci, int num)
{
	if (*pci == 'x' || *pci == 'X') {
		return hex2int(pci + 1, 4);
	}
	else {
		return atoi(pci);
	}
}

void changeunicode(char *inbuff,int *insize,ConvertCtx *apCtx,int isunicode,int basecode)
{
	int  no,num;	
	char unicode[64];
	char *pold,*pnew;
	
	int  size;
	char *p,*buff;
	
	unicode[5]='\0';
	
	if (!isunicode || !*insize)
		return;
	
	no=apCtx->unicodeno;
	size=*insize+no;
	if (no)
	{
		if (size > apCtx->nbakunicode) {
			int ncu = size;
			if (ncu < 4096) {
				ncu = 4096;
			}
			apCtx->nbakunicode = ncu;
			apCtx->pchbakunicode = apr_palloc(apCtx->p, LOGALLOCSIZE(apCtx->nbakunicode));
		}
		buff = apCtx->pchbakunicode;
		memcpy(buff,apCtx->unicodeinfo,no);
		memcpy(buff+no,inbuff,*insize);				
		apCtx->unicodeno=0;
	}
	else
		buff=inbuff; 	
	
	if (*(buff+size-1)=='&')
	{
		apCtx->unicodeinfo[0]='&';
		apCtx->unicodeno=1;
		size-=1;
	}
	else
	{
		for(p=buff+size-1;p>buff;p--)
			if (((*(p-1)=='&') && (*p=='#')) || (p+7<=buff+size))
				break;
			
			if ((p!=buff) && (p+7>buff+size))
			{
				memcpy(apCtx->unicodeinfo,p-1,buff+size-p+1);
				apCtx->unicodeno=buff+size-p+1;
				size=p-buff-1;
			}
	}
	
	pnew=pold=buff;
	
	while(pold<=buff+size-8)
	{
		if ((*pold=='&') && (*(pold+1)=='#') && (*(pold+7)==';'))
		{
			memcpy(unicode,pold+2,5);			
			num = hz_unicode(unicode, 5);
			pold+=8;
			if (basecode==ENCODE_GB2312)
			{
				*pnew++=untogb[num][0];
				*pnew++=untogb[num][1];			
			}
			else if (basecode==ENCODE_BIG5)
			{
				*pnew++=untobig[num][0];
				*pnew++=untobig[num][1];			
			}
		}
		else
			*pnew++=*pold++;
	}
	
	while(pold<buff+size)
		*pnew++=*pold++;
	
	*insize=pnew-buff;
	
	if (buff!=inbuff)
	{
		memcpy(inbuff,buff,*insize);		
	}
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int hz_hex(char *pci, int num)
{
	int hex = 0;
	char *ptr = pci, *pend = pci + num;
	
	while (ptr < pend) {
		hex <<= 4;
		if (*ptr >= '0' && *ptr <= '9') {
			if (hex == 0) {
				return 0;
			}
			hex += *ptr++ - 48;
		}
		else if (*ptr >= 'a' && *ptr <= 'f') {
			hex += *ptr++ - 87;
		}
		else if (*ptr >= 'A' && *ptr <= 'F') {
			hex += *ptr++ - 55;
		}
		else {
			return 0;
		}
	}
	
	return hex;
}

int IsHexChar(char *pci, int num)
{
	int hex = 0;
	char *ptr = pci, *pend = pci + num;
	
	while (ptr < pend) {
		hex <<= 4;
		if (*ptr >= '0' && *ptr <= '9') {
			hex += *ptr++ - 48;
		}
		else if (*ptr >= 'a' && *ptr <= 'f') {
			hex += *ptr++ - 87;
		}
		else if (*ptr >= 'A' && *ptr <= 'F') {
			hex += *ptr++ - 55;
		}
		else return 0;
	}
	
	return hex > 10000 ? hex : 0;
} 

int hz_unescape(char *inbuf, int *size, int encode)
{
	int hz;
	char *ptr, *pbeg, *pend;
	
	ptr = inbuf;
	pbeg = inbuf;
	pend = inbuf + *size;
	while (ptr + 2 < pend) {
		if (*ptr == '%' && ((hz = hz_hex(ptr + 1, 2)) != 0)) {			
			*pbeg++ = hz & 0xFF;
			ptr += 3;
			continue;
		}
		if (*ptr == '\\' && *(ptr + 1) == 'u' && ptr + 5 < pend && ((hz = IsHexChar(ptr + 2, 4)) != 0)) {
			if (encode == ENCODE_GB2312) {
				*pbeg++ = untogb[hz][0];
				*pbeg++ = untogb[hz][1];
			}
			else {
				*pbeg++ = untobig[hz][0];
				*pbeg++ = untobig[hz][1];
			}
			ptr += 6;
			continue;
		}
		*pbeg++ = *ptr++;		
	}
	while (ptr < pend) {
		*pbeg++ = *ptr++;
	}
	*size = pbeg - inbuf;
	
	return *size;
}


// "%e7%9c%81%e4%bc%9a%e5%9f%8e%e5%b8%82"; UTF8 encode chinese ... unescape 
int unescape_utf8(char **pout, char *pin, int size, ConvertCtx *apCtx, int encode)
{
	int hz = 0;
	int i = 0, j = 0;
	char *puse = NULL;
	char *pbuffer = NULL;
	
	while (i < size) {
		if (i % 3 == 0) {
			if (*(pin + i) == '%') {				
				NULL;
			}
			else {
				break;
			}
		}
		else {
			if (*(pin + i) >= '0' && *(pin + i) <= '9') {
				NULL;
			}
			else if (*(pin + i) >= 'a' && *(pin + i) <= 'f') {
				NULL;
			}
			else if (*(pin + i) >= 'A' && *(pin + i) <= 'F') {
				NULL;
			}
			else {
				break;
			}
		}
		++i;
	}
	i -= i % 3;
	if (i < 1) {
		return 0;
	}
	pbuffer = apr_palloc(apCtx->p, LOGALLOCSIZE(i + 64));	
	puse = pbuffer;
	while (j < i) {
		if (j % 3 == 0) {
			hz = 0;
			if (*(pin + j) == '%') {				
				NULL;
			}
			else {
				break;
			}
		}
		else {
			hz <<= 4;
			if (*(pin + j) >= '0' && *(pin + j) <= '9') {
				hz += *(pin + j) - '0';
				NULL;
			}
			else if (*(pin + j) >= 'a' && *(pin + j) <= 'f') {
				hz += *(pin + j) - 'a' + 10;
				NULL;
			}
			else if (*(pin + j) >= 'A' && *(pin + j) <= 'F') {
				hz += *(pin + j) - 'A' + 10;
				NULL;
			}
			else {
				break;
			}
			if (j % 3 == 2) {
				*puse++ = hz;
			}
		}
		++j;
	}
	j = puse - pbuffer;
	if (j < 2) {
		return 0;
	}
	j = ConvertFromUTF8Ex(pbuffer, j, encode, apCtx);
	if (j < 1) {
		return 0;
	}
	memcpy(*pout, pbuffer, j);
	*pout += j;
	
	return i;
}


int my_unescape_ex(char **p2inbuf, int *size, config *pconfig, ConvertCtx *apCtx, int encode)
{
	int n, unicode;
	int num = *size;
	char *inbuf = *p2inbuf;
	char *pbuf, *ptr, *pbak, *pend;
	
	if (apCtx->nHexChar > 0) {
		pbuf = apr_palloc(apCtx->p, LOGALLOCSIZE(num + 10));
		memcpy(pbuf, apCtx->chHexChar, apCtx->nHexChar);
		memcpy(pbuf + apCtx->nHexChar, inbuf, num);
		num += apCtx->nHexChar;
		apCtx->chHexChar[0] = '\0';
		apCtx->nHexChar = 0;
	}
	else {
		pbuf = inbuf;
	}
	ptr = pbak = pbuf;
	pend = pbuf + num;
	
	while (ptr < pend) {
		unicode = 0;
		n = pend - ptr;		
		if (*ptr == '%') {
			if (n < 3) {
				memcpy(apCtx->chHexChar, ptr, n);
				apCtx->chHexChar[n] = '\0';
				apCtx->nHexChar = n;                
				break;
			}
			else if (pconfig->m_iEscapeUTF8Char == 1 
				&& (unicode = unescape_utf8(&pbak, ptr, n, apCtx, encode)) > 0) 
			{				
				ptr += unicode;
				continue;
			}			
			else if (pconfig->m_iUnescapeChar > 2 && (unicode = hz_hex(ptr + 1, 2)) != 0) {
				*pbak++ = unicode & 0xFF;
				ptr += 3;
				continue;
			}			
			if (n < 6) {
				memcpy(apCtx->chHexChar, ptr, n);
				apCtx->chHexChar[n] = '\0';
				apCtx->nHexChar = n;                
				break;
			}
			else if (*(ptr + 1) == 'u') {
				if ((unicode = IsHexChar(ptr + 2, 4)) < 1) {
					memcpy(pbak, ptr, 6);
					pbak += 6;
					ptr += 6;				
					continue;
				}
				else {
					ptr += 6;
				}
			}
			else if (*(ptr + 1) == '2' && *(ptr + 2) == '5' && *(ptr + 3) == 'u') {
				if (n < 8) {
					memcpy(apCtx->chHexChar, ptr, n);
					apCtx->chHexChar[n] = '\0';
					apCtx->nHexChar = n;                    
					break;
				}
				if ((unicode = IsHexChar(ptr + 4, 4)) < 1) {
					memcpy(pbak, ptr, 8);
					pbak += 8;
					ptr += 8;
					continue;
				}
				else {
					ptr += 8;
				}
			}
			else {
				*pbak++ = *ptr++;
				continue;
			}
		}
		else if (pconfig->m_iUnescapeChar > 1 && *ptr == '\\') {
			if (n < 6) {
				memcpy(apCtx->chHexChar, ptr, n);
				apCtx->chHexChar[n] = '\0';
				apCtx->nHexChar = n;                
				break;
			}
			else if (*(ptr + 1) == '\\' && *(ptr + 2) == 'u') { // wpf \\u6ca1\\u6709\\u5339  
				if ((unicode = IsHexChar(ptr + 3, 4)) < 1) {
					memcpy(pbak, ptr, 6);
					pbak += 6;
					ptr += 6;
					continue;
				}
				else {
					ptr += 7;
				}
			}
			else if (*(ptr + 1) == 'u') {
				if ((unicode = IsHexChar(ptr + 2, 4)) < 1) {
					memcpy(pbak, ptr, 6);
					pbak += 6;
					ptr += 6;				
					continue;
				}
				else {
					ptr += 6;
				}
			}
			else {
				*pbak++ = *ptr++;
				continue;
			}
		}
		else {
			*pbak++ = *ptr++;
			continue;
		}
		if (unicode > 0) {                 
			if (encode == ENCODE_GB2312) {
				*pbak++ = untogb[unicode][0];
				*pbak++ = untogb[unicode][1];
			}
			else {
				*pbak++ = untobig[unicode][0];
				*pbak++ = untobig[unicode][1];
			}
		}		
	}
	*p2inbuf = pbuf;
	*size = pbak - pbuf;
	
	return *size;
}



int my_unescape(char *inbuf, int *size, ConvertCtx *apCtx, int encode)
{
	int unicode;
	int num = *size;
	char sbuf[4096]; 
	char *pbak, *pbuf;
	char *ptr = inbuf, *pend; 
	
	int ret = 0; 
	
	if (strnistr(inbuf, "%", num) == NULL) {	
		return 0;
	}
	
	if (num + 10 < sizeof(sbuf)) {
		pbuf = sbuf;
	}
	else {
		pbuf = apr_palloc(apCtx->p, LOGALLOCSIZE(num + 10));
	}
	
	if (apCtx->nHexChar > 0) {
		memcpy(pbuf, apCtx->chHexChar, apCtx->nHexChar);
		memcpy(pbuf + apCtx->nHexChar, inbuf, num);
		num += apCtx->nHexChar;
		apCtx->nHexChar = 0;
		apCtx->chHexChar[0] = '\0';
	}
	else {
		memcpy(pbuf, inbuf, num);
	}
	pbak = pbuf;
	pend = pbuf + num;
	
	while (pbak < pend) {
		if (*pbak == '%') {
			int n = pend - pbak;
			unicode = 0;
			if (n < 6) {
				memcpy(apCtx->chHexChar, pbak, n);
				apCtx->nHexChar = n;
				apCtx->chHexChar[n] = '\0';
				break;
			}
			else if (*(pbak + 1) == 'u') {
				unicode = IsHexChar(pbak + 2, 4);
				if (unicode < 1) {
					memcpy(ptr, pbak, 6);
					ptr += 6;				
				}
				pbak += 6;				
			}
			else if (*(pbak + 1) == '2' && *(pbak + 2) == '5' && *(pbak + 3) == 'u') {
				if (n < 8) {
					memcpy(apCtx->chHexChar, pbak, n);
					apCtx->nHexChar = n;
					apCtx->chHexChar[n] = '\0';
					break;
				}
				unicode = IsHexChar(pbak + 4, 4);
				if (unicode < 1) {
					memcpy(ptr, pbak, 8);
					ptr += 8;
				}
				pbak += 8;
			}
			else {
				*ptr++ = *pbak++;
			}
			if (unicode > 0) { 
				if (ret == 0) {
					ret = 1; 
					apCtx->nUrlEncode = encode;
				}
				if (encode == ENCODE_GB2312) {
					*ptr++ = untogb[unicode][0];
					*ptr++ = untogb[unicode][1];
				}
				else {
					*ptr++ = untobig[unicode][0];
					*ptr++ = untobig[unicode][1];
				}
			}
		}
		else {
			*ptr++ = *pbak++;
		}
	} 
	
	{// WPF 2003-5-23 not backup left data  
		int n = apCtx->nHexChar; 
		
		if (n > 0) {
			memcpy(ptr, apCtx->chHexChar, n); 
			ptr += n; 
			apCtx->nHexChar = 0; 
			apCtx->chHexChar[0] = '\0'; 
		}
	} 
	
	*size = ptr - inbuf; 
	return ret;
}

int my_escape(int fromcode, char *pin, int nsize, char *pout)
{
	char *puse = pin;
	char *pbak = pout;
	char *pend = pin + nsize;
	
	*pout = '\0';
	
	while (puse < pend) {
		if (*puse & 0x80) {
			int n = 0;
			char buff[64];
			
			n = ConvertToUnicodeExt(fromcode, FEFF, 0, puse, 2, buff);
			puse += 2;
			
			if (n == 2) {
				unsigned char h, ch;
				
				*pbak++ = '%';
				*pbak++ = 'u';
				
				ch = buff[0];
				h = ch / 16;
				if (h < 10) {
					h += 48;
				}
				else {
					h += 55;
				}
				*pbak++ = h; 				
				ch = buff[0];
				h = ch % 16;
				if (h < 10) {
					h += 48;
				}
				else {
					h += 55;
				}
				*pbak++ = h;
				
				ch = buff[1];
				h = ch / 16;
				if (h < 10) {
					h += 48;
				}
				else {
					h += 55;
				}
				*pbak++ = h; 				
				ch = buff[1];
				h = ch % 16;
				if (h < 10) {
					h += 48;
				}
				else {
					h += 55;
				}
				*pbak++ = h; 
			}
		}
		else {
			*pbak++ = *puse++; 
		}
	} 
	
	*pbak = '\0';
	return pbak - pout; 
} 

/*-----------------------------------------------------------------------------------------------------------------------*/

int inithkword(hkword *hktounicode[256][256],char *filename, pool *p)
{
	FILE *fp;
	int  i,j;
	int  n,k;
	char *ptr;	
	char str[64],line[512];
	unsigned char ch1,ch2,ch3,ch4;
	hkword *temp;
	
	if ((fp=fopen(filename,"rb"))==NULL)
		return(0);
	
	for(i=0;i<256;i++)
		for(j=0;j<256;j++)
			hktounicode[i][j]=NULL;
		
		while (!feof(fp))
		{
			if (!fgets(line,sizeof(line),fp))
				break;
			
			if (!(ptr= strchr(line, '\t')))
				continue;		
			
			ch1=(unsigned char)line[0];
			ch2=(unsigned char)line[1];
			ch3=(unsigned char)line[3];
			ch4=(unsigned char)line[4];
			
			k = ch3*256 + ch4;
			memcpy(str,"&#",2);
			itoa(k,str+2,10);
			strcat(str,";");
			
			ptr=line+6;
			while((*ptr>='0') && (*ptr<='9'))
				ptr++;		
			*ptr='\0';		
			ptr=line+6;
			n=atoi(ptr);
			
			temp=(hkword*)apr_palloc(p, sizeof(hkword));
			temp->group = n;
			strcpy(temp->unicode,str);
			hktounicode[ch1][ch2]=temp;		
		} 
		fclose(fp); 
		
		return(1);
}

int makefont_eot(char *string, char *prefix, int num)
{
	static char addfont[]     = "<STYLE TYPE=\"text/css\">\r\n<!--\r\n@font-face\r\n{\r\n  font-family: DFPMingLight-B";
	static char addfontmid[]  = ";\r\n  font-style : normal;\r\n  font-weight: normal;\r\n  src: url(";
	static char addfontlast[] = ".eot);\r\n}\r\n-->\r\n</STYLE>";
	
	int  len;
	char *ptr = string;
	char numstr[10];
	int  ipre = strlen(addfont);
	int  imid = strlen(addfontmid);
	int  ilst = strlen(addfontlast);
	
	memset(numstr, '\0', sizeof(numstr));
	memset(string, '\0', sizeof(string));
	itoa(num, numstr, 10);
	len = strlen(numstr);
	memcpy(ptr, addfont, ipre);	
	ptr += ipre;
	memcpy(ptr, numstr, len);
	ptr += len;
	memcpy(ptr, addfontmid, imid);
	ptr += imid;
	memcpy(ptr, prefix, strlen(prefix));
	ptr += strlen(prefix);
	memcpy(ptr, numstr, len);
	ptr += len;
	memcpy(ptr, addfontlast, ilst);
	*(ptr +ilst) = '\0';
	return (ptr + ilst - string);
}

int makefont_pfr(char *string, char *prefix, int num, ConvertCtx *locaptr)
{
	char head1[] = "<LINK REL=\"FONTDEF\" SRC=\"";
	char tail1[] = ".pfr\">";
	char head6[] = "<EMBED URL=\"";
	char tail6[] = ".wpf\" TYPE=application/x-info-plugin HIDDEN height=0px width=0px>";
	char *head = head1, *tail = tail1;
	char temp[64];
	char *ptr = string;
	int  nlen;
	
	if (locaptr->nIsNetscape == 6)
	{
		head = head6;
		tail = tail6;
	}
	
	if (locaptr->nAddPlugin == 0)
	{
		char s[1024];
		char s1[] = "<script src=\"";
		char s2[] = "info.js\"></script><script>CheckPlugin(\"";
		char s3[] = "\");</script>";
		
		strcpy(s, s1);
		strcat(s, prefix);
		strcat(s, s2);
		strcat(s, prefix);
		strcat(s, s3);
		
		strcpy(ptr, s);
		ptr += strlen(s);			
		locaptr->nAddPlugin = 1;
	}
	
	nlen = strlen(head);
	memcpy(ptr, head, nlen);
	ptr += nlen;
	nlen = strlen(prefix);
	memcpy(ptr, prefix, nlen);
	ptr += nlen;
	itoa(num, temp, 10);
	nlen = strlen(temp);
	memcpy(ptr, temp, nlen);
	ptr += nlen;
	nlen = strlen(tail);
	memcpy(ptr, tail, nlen);
	ptr += nlen;
	*ptr = '\0';
	
	return ptr - string;
}

int makefont(char *string, char *prefix, int num, ConvertCtx *locaptr)
{
	if (locaptr->nIsNetscape)
		return makefont_pfr(string, prefix, num, locaptr);
	else
		return makefont_eot(string, prefix, num);
}

int makeword_eot(char *string,unsigned char ch1,unsigned char ch2,int num)
{
	static char prefont[]     = "<font face=\"DFPMingLight-B";
	static char prefontlast[] = "\">";
	static char sucfont[]     = "</font>";
	
	int  len;
	char *ptr = string;
	char numstr[10];
	int  lenpre = strlen(prefont);
	int  lenlst = strlen(prefontlast);
	int  lensuc = strlen(sucfont);
	
	memset(numstr, '\0', sizeof(numstr));
	memset(string, '\0', sizeof(string));
	memcpy(ptr, prefont, lenpre);
	ptr += lenpre;
	itoa(num, numstr, 10);
	len = strlen(numstr);
	memcpy(ptr, numstr, len);
	ptr += len;
	memcpy(ptr, prefontlast, lenlst);
	ptr += lenlst;
	len = strlen(hktogb[ch1][ch2]->unicode);
	memcpy(ptr,hktogb[ch1][ch2]->unicode,len);
	ptr += len;
	memcpy(ptr, sucfont, lensuc);	
	*(ptr + lensuc) = '\0';
	return (ptr + lensuc - string);
}

int makeword_pfr(char *string,unsigned char ch1,unsigned char ch2,int num)
{
	char left[] = "<font face=\"info";
	char mid[] = "\">";
	char right[] = "</font>";
	char temp[64];
	char *ptr = string;
	int nlen;
	
	nlen = strlen(left);
	memcpy(ptr, left, nlen);
	ptr += nlen;
	// itoa(num, temp, 10);
	itoa(num + 1, temp, 10);
	nlen = strlen(temp);
	memcpy(ptr, temp, nlen);
	ptr += nlen;
	nlen = strlen(mid);
	memcpy(ptr, mid, nlen);
	ptr += nlen;
	nlen = strlen(hktogb[ch1][ch2]->unicode);
	memcpy(ptr, hktogb[ch1][ch2]->unicode, nlen);
	ptr += nlen;
	nlen = strlen(right);
	memcpy(ptr, right, nlen);
	ptr += nlen;
	*ptr = '\0';
	
	return ptr - string;
}

int makeword(char *string,unsigned char ch1,unsigned char ch2,int num, int isns)
{
	if (isns)
		return makeword_pfr(string, ch1, ch2, num);
	else
		return makeword_eot(string, ch1, ch2, num);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

/*  
If the parameter twobytes is 0 then unicode format is &#12345;
else if twobytes is FFFE then unicode format is FFFE
else unicode format is FEFF
*/
int utf82unicode(char *pinput, int insize, char *pout, ConvertCtx *apCtx, int twobytes)
{
	char sbuf[8192];
	char *puse, *padd;
	char *pbak = pout;
	char temp[64] = {0};
	char str[64]  = {'&', '#'};
	int  unicode, chhead, chmid, chtail;
	
	if (apCtx->utf8no > 0)
	{
		if (insize + apCtx->utf8no < sizeof(sbuf)) {
			padd = sbuf;
		}
		else {
			padd = apr_palloc(apCtx->p, LOGALLOCSIZE(insize+apCtx->utf8no));
		}
		memcpy(padd, apCtx->utf8info, apCtx->utf8no);
		memcpy(padd+apCtx->utf8no, pinput, insize);		
		insize += apCtx->utf8no;
		apCtx->utf8no = 0;
	}
	else
		padd = pinput;
	
	puse = padd;
	
	while(puse < padd+insize)
	{
		if ((*puse & 0x80) == 0)
		{
			if (twobytes == FEFF) {
				*pbak++ = 0;
			}
			*pbak++ = *puse++;
			if (twobytes == FFFE) {
				*pbak++ = 0;
			}
		}
		else
		{
			int len;
			chhead = (int)*puse++;
			if ((chhead & 0xE0) == 0xC0)
			{
				if (puse == padd+insize)
				{
					memcpy(apCtx->utf8info, puse-1, sizeof(char));
					apCtx->utf8no = sizeof(char);
					break;
				}
				chtail  = (int)*puse++;
				unicode = (chhead & 0x1F) << 6 | (chtail & 0x3F);
			}
			else
			{
				if (puse > padd+insize-2)
				{
					memcpy(apCtx->utf8info, puse-1, sizeof(char)*(padd+insize-puse)+1);
					apCtx->utf8no = sizeof(char)*(padd+insize-puse)+1;
					break;				
				}
				chmid   = (int)*puse++;
				chtail  = (int)*puse++;
				unicode = (chhead & 0x0F) << 12 | (chmid & 0x3F) << 6 | (chtail & 0x3F);			
			}
			
			if (twobytes == 0)
			{
				itoa(unicode, temp, 10);
				strcat(temp, ";");
				strcpy(str+2, temp);
				len = strlen(str);
				memcpy(pbak, str, len);
				pbak += len;
			}
			else if (twobytes == FEFF)
			{
				*pbak++ = (char)(unicode >> 8);
				*pbak++ = (char)unicode;
			}
			else {
				*pbak++ = (char)unicode;
				*pbak++ = (char)(unicode >> 8);                
			}
		}
	}
	
	return pbak - pout;
}

/* Unicode format is two bytes */
int unicode2utf8(char *pins, int nsize, int exchange, char *pout)
{
	int num;
	unsigned char chh, chl;
	char *ptr = pins, *puse = pout;
	
	if (nsize % 2) {
		return 0;
	}
	
	while (ptr < pins + nsize) {
		chh = (unsigned char)*ptr++;
		chl = (unsigned char)*ptr++;        
		if (exchange) {
			num = chl * 256 + chh;
		}
		else {
			num = chh * 256 + chl;
		}
		
		if (num < 0x80) {
			*puse++ = num >> 0 & 0x7F | 0x00;
		}
		else if (num < 0x0800) {
			*puse++ = num >> 6 & 0x1F | 0xC0;
			*puse++ = num >> 0 & 0x3F | 0x80;
		}
		else if (num < 0x010000) {
			*puse++ = num >> 12 & 0x0F | 0xE0;
			*puse++ = num >> 6 & 0x3F | 0x80;
			*puse++ = num >> 0 & 0x3F | 0x80;
		}
		else if (num <0x110000) {
			*puse++ = num >> 18 & 0x07 | 0xF0;
			*puse++ = num >> 12 & 0x3F | 0x80; 
			*puse++ = num >> 6 & 0x3F | 0x80; 
			*puse++ = num >> 0 & 0x3F | 0x80; 
		}
		
	}
	
	return puse - pout;
}

int my_atoi(char *str)
{
	int r = 0;
	
	while (*str >= '0' && *str <= '9') {
		r *= 10;
		r += *str - '0';
		++str;
	}
	
	return r;
}

int url_unicode2utf8(char *url, char *newurl)
{
	int flag = 0;
	int i, n, hz;
	char *puse = newurl;
	char *pend = url + strlen(url);
	char utf8[64], unicode[64];
	
	while (*url != '\0') {
		if (*url == '&' && (url + 7) < pend) {
			if (*(url + 1) == '#' && *(url + 7) == ';') {
				hz = my_atoi(url + 2);
				unicode[0] = hz >> 8;
				unicode[1] = hz & 0xFF;
				n = unicode2utf8(unicode, 2, 0, utf8);
				for (i = 0; i < n; i++) {
					sprintf(puse, "%%%02X", utf8[i] & 0xFF);
					puse += 3;
				}
				if (n > 0) {
					url += 8;
					flag = 1;
					continue;
				}
			}
		}
		*puse++ = *url++;
	}
	*puse = '\0';
	
	if (flag == 0) {
		return 0;
	}
	return puse - newurl;
}

/*-----------------------------------------------------------------------------------------------------------------------*/
/* Change betwen ANSI and UNICODE */

int ConvertFromUnicode(char *pTable, int flag, char *pIns, int nSize, char *pPut)
{
	int  nNum, nNext = sizeof(char)*2;
	char *ptr, *pUse = pIns, *pOut = pPut;
	unsigned char chHigh, chLow;
	
	while(pUse < pIns+nSize)
	{
		chHigh = (unsigned char)*pUse++;
		if (pUse == pIns+nSize)
		{
			*pOut++ = *(pUse-1);
			break;
		}
		chLow = (unsigned char)*pUse++;
		
		if (flag == FFFE)
			nNum = chLow*256 + chHigh;
		else
			nNum = chHigh*256 + chLow;
		
		if (nNum < 0x80)
			*pOut++ = (char)nNum;
		else
		{
			ptr = pTable + nNext*nNum;					
			if (*ptr != '\0' || *(ptr + 1) != '\0') {
				*pOut++ = *ptr++;
				*pOut++ = *ptr;
			}
		}
	}
	
	return pOut-pPut;
}

int make_unicode(char *pbuff, int len, int *code)
{
	int i = 0;
	int hz = 0;
	
	if (len < 8) {
		return 0;
	}
	if (*pbuff != '&' || *(pbuff + 1) != '#') {
		return 0;
	}
	pbuff += 2;
	for (i = 0; i < 6; i++) {
		if (*(pbuff + i) == ';') {
			hz = atoi(pbuff);
			break;
		}
		else if (*(pbuff + i) < '0' || *(pbuff + i) > '9') {
			break;
		}
	}
	if (hz > 0 && hz < 0xFFFF) {
		*code = hz;
		return i + 3;
	}
	
	return 0;
}

int ConvertToUnicode(char *pTable, int flag, char *pIns, int nSize, char *pPut)
{
	int  nNum, nNext = sizeof(char) * 2;
	char *ptr, *pUse = pIns, *pOut = pPut;
	char *pend = pIns + nSize;
	unsigned char chHigh, chLow;
	
	while(pUse < pend) {
		chHigh = (unsigned char) *pUse++;        
		if (chHigh & 0x80) {
			if (pUse == pend) {
				*pOut++ = *(pUse - 1);
				break;
			}
			else {
				chLow = (unsigned char) *pUse++;
			}            
			nNum = chHigh * 256 + chLow;
			ptr =  pTable + nNext * nNum;
			chHigh = (unsigned char) *ptr++;
			chLow = (unsigned char) *ptr;
		}
		else {
			int n = make_unicode(pUse - 1, pend - pUse + 1, &nNum);
			if (n > 0) {
				chHigh = (unsigned char) ((nNum >> 8) & 0xFF);
				chLow = (unsigned char) (nNum & 0xFF); 
				pUse += n - 1;
			}
			else {
				chLow = chHigh;
				chHigh = 0;
			}
		}        
		if (flag == FFFE) {
			*pOut++ = (char)chLow;
			*pOut++ = (char)chHigh;
		}
		else {
			*pOut++ = (char)chHigh;
			*pOut++ = (char)chLow;			
		}        
	}
	
	return pOut - pPut;
}

/*-----------------------------------------------------------------------------------------------------------------------*/
/* Convert gb2312 or big5 to %uA8A9 encoding
* flag is FFFE or FEFF
* format 0 unicode is two bytes format 1 unicode is &#12345;
*/
int ConvertToEscapeUnicode(int fromcode, int flag, int format, char *pIns, int nSize, char *pPut)
{
	char *pTable = NULL;
	
	if (fromcode == ENCODE_GB2312)
	{
		pTable = gb2unicode;
	}
	else
	{
		pTable = big2unicode;
	}
	
	{
		int  nNum, nNext = sizeof(char)*2;
		char *ptr, *pUse = pIns, *pOut = pPut;
		unsigned char chHigh, chLow;
		
		while(pUse < pIns + nSize)
		{
			chHigh = (unsigned char)*pUse++;
			
			if (chHigh & 0x80)
			{
				unsigned int word;
				
				if (pUse == pIns+nSize)
				{
					*pOut++ = *(pUse-1);
					break;
				}
				else
					chLow = (unsigned char)*pUse++;
				
				nNum = chHigh*256 + chLow;
				ptr =  pTable + nNext*nNum;
				word = ((unsigned char)*ptr) * 256 + (unsigned char)*(ptr + 1);
				
				if (word)
				{
					char len, str[128] = {'%', 'u'};
					
					if (format == 2) {
						sprintf(str, "\\u%x", word);					
					}
					else {
						sprintf(str, "%%u%x", word);					
					}
					len = strlen(str);
					memcpy(pOut, str, len);
					pOut += len;					
				}
				else
				{
					*pOut++ = (char)chHigh;
					*pOut++ = (char)chLow;
				}
			}
			else
			{
				*pOut++ = (char)chHigh;
			}						
		}
		
		return pOut - pPut;
	}
}

/*-----------------------------------------------------------------------------------------------------------------------*/
/* Convert gb2312 or big5 to unicode encoding
* flag is FFFE or FEFF
* format 0 unicode is two bytes format 1 unicode is &#12345;
*/

int ConvertToUnicodeExt(int fromcode, int flag, int format, char *pIns, int nSize, char *pPut)
{
	char *pTable = NULL;
	
	if (fromcode == ENCODE_GB2312)
	{
		pTable = gb2unicode;
	}
	else
	{
		pTable = big2unicode;
	}
	
	if (!format)
	{
		return 	ConvertToUnicode(pTable, flag, pIns, nSize, pPut);
	}
	else
	{
		int  nNum, nNext = sizeof(char)*2;
		char *ptr, *pUse = pIns, *pOut = pPut;
		unsigned char chHigh, chLow;
		
		while(pUse < pIns + nSize)
		{
			chHigh = (unsigned char)*pUse++;
			
			if (chHigh & 0x80)
			{
				unsigned int word;
				
				if (pUse == pIns+nSize)
				{
					*pOut++ = *(pUse-1);
					break;
				}
				else
					chLow = (unsigned char)*pUse++;
				
				nNum = chHigh*256 + chLow;
				ptr =  pTable + nNext*nNum;
				word = ((unsigned char)*ptr) * 256 + (unsigned char)*(ptr + 1);
				
				if (word)
				{
					char len, str[128] = {'&', '#'};
					
					itoa(word, &str[2], 10);
					len = strlen(str);
					memcpy(pOut, str, len);
					pOut += len;
					*pOut++ = ';';
				}
				else
				{
					*pOut++ = (char)chHigh;
					*pOut++ = (char)chLow;
				}
			}
			else
			{
				*pOut++ = (char)chHigh;
			}						
		}
		
		return pOut - pPut;
	}
}

int ConvertFromUnicodeEx(int fromcoding, int flag, char *pIns, int nSize, char *pPut)
{
	char *pTable = NULL;
	
	if (fromcoding == ENCODE_GB2312)
		pTable = unicode2gb;
	else
		pTable = unicode2big;
	
	return ConvertFromUnicode(pTable, flag, pIns, nSize, pPut);
}

/*-----------------------------------------------------------------------------------------------------------------------*/
/* Use for ConvertToUnicode(...) and ConvertFromUnicode(...) function */
/* The ForUnicode(...) import from exportapi.c */
/* The Unicode table format is &#12345; */

int initunicode_no2(pool *p)
{
	int buffsize = 256 * 256 * 2;
	char buf[512];
	
	buf[0] = '\0';
	unicode2gb = (char *)apr_palloc(p, buffsize);
	unicode2big = (char *)apr_palloc(p, buffsize);
	gb2unicode = (char *)apr_palloc(p, buffsize);
	big2unicode = (char *)apr_palloc(p, buffsize);
	memset(unicode2gb, 0, buffsize);
	memset(unicode2big, 0, buffsize);
	memset(gb2unicode, 0, buffsize);
	memset(big2unicode, 0, buffsize);
	
#ifndef WIN32
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/unicodetogb.txt");
	if (!ForUnicode(unicode2gb, gb2unicode, buf))
		return 0;
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/unicodetobig.txt");
	if (!ForUnicode(unicode2big, big2unicode, buf))
		return 0;
#else
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\unicodetogb.txt");
	if (!ForUnicode(unicode2gb, gb2unicode, dir))
		return 0;
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\unicodetobig.txt");
	if (!ForUnicode(unicode2big, big2unicode, dir))
		return 0;
#endif
	
	return 1;
}

/*-----------------------------------------------------------------------------------------------------------------------*/
/* Change betwen ANSI and UTF-8 */


char* ConvertFromUTF8_auto_unicode(char *pins, int insize, config *pcon, ConvertCtx *pctx, int *outSize)
{
	int n = 0;
	char sbuf[2048];
	char *putf, *ptable;
	
	*outSize = 0;	
	if (insize * 2 + 64 < sizeof(sbuf)) {
		putf = sbuf;
	}
	else {
		putf = apr_palloc(pctx->p, LOGALLOCSIZE(insize * 2 + 64));
	}	
	n = utf82unicode(pins, insize, putf, pctx, FEFF);	
	if (pcon->m_iFromEncode == ENCODE_GB2312) {
		ptable = unicode2gb;
	}
	else {
		ptable = unicode2big;
	}	
	//	n = ConvertFromUnicode(ptable, FEFF, putf, n, pins);	
	{
		int first = 1;
		int flag = FEFF;
		int nNum, nNext = sizeof(char)*2;
		char *ptr, *pUse = putf, *pOut = pins;
		unsigned char chHigh, chLow;
		
		while(pUse < putf+n) {
			chHigh = (unsigned char)*pUse++;
			if (pUse == putf + n) {
				*pOut++ = *(pUse-1);
				break;
			}
			chLow = (unsigned char)*pUse++;			
			if (flag == FFFE) {
				nNum = chLow*256 + chHigh;
			}
			else {
				nNum = chHigh*256 + chLow;
			}			
			if (nNum < 0x80) {
				*pOut++ = (char)nNum;
			}
			else {
				ptr = ptable + nNext*nNum;					
				if (*ptr != '\0' || *(ptr + 1) != '\0') {
					*pOut++ = *ptr++;
					*pOut++ = *ptr;
				}
				else if (pcon->m_iAutoUseUnicode == 1) {
					if (first == 1) {
						char *temp = apr_palloc(pctx->p, LOGALLOCSIZE(n * 4 + 64));
						memcpy(temp, pins, pOut - pins);
						pOut = temp + (pOut - pins);
						pins = temp;
						first = 0;							
					}
					if (nNum == 0xFFFE || nNum == 0xFEFF) {
						continue;
					}
					{
						char unicode[32] = {'\0'};
						
						itoa(nNum, unicode, 10);
						*pOut++ = '&';
						*pOut++ = '#';
						memcpy(pOut, unicode, strlen(unicode));
						pOut += strlen(unicode);
						*pOut++ = ';';				
					}
				}
			}
		}		
		n = pOut - pins;
	}	
	*outSize = n;
	
	return pins;
}


int ConvertFromUTF8(char *pins, int insize, config *pcon, ConvertCtx *pctx)
{
	int n;
	char sbuf[2048];
	char *putf, *ptable;
	
	if (insize * 2 + 64 < sizeof(sbuf)) {
		putf = sbuf;
	}
	else {
		putf = apr_palloc(pctx->p, LOGALLOCSIZE(insize * 2 + 64));
	}
	
	n = utf82unicode(pins, insize, putf, pctx, FEFF);
	
	if (pcon->m_iFromEncode == ENCODE_GB2312)
		ptable = unicode2gb;
	else
		ptable = unicode2big;
	
	n = ConvertFromUnicode(ptable, FEFF, putf, n, pins);
	
	return n;
}

int ConvertFromUTF8Ex(char *pins, int insize, int FromEncode, ConvertCtx *pctx)
{
	int n;
	char sbuf[2048];
	char *putf, *ptable;
	
	if (insize * 3 < sizeof(sbuf)) {
		putf = sbuf;
	}
	else {
		putf = apr_palloc(pctx->p, LOGALLOCSIZE(insize * 3));
	}
	
	n = utf82unicode(pins, insize, putf, pctx, FEFF);
	
	if (FromEncode == ENCODE_GB2312)
		ptable = unicode2gb;
	else
		ptable = unicode2big;
	
	n = ConvertFromUnicode(ptable, FEFF, putf, n, pins);
	
	return n;
}

int ConvertToUTF8(char *pins, int insize, char *pout, config *pcon, pool *p)
{
	int num;
	char sbuf[8192];
	char *usetable, *ptemp = NULL;
	
	if (insize * 2 < sizeof(sbuf)) {
		ptemp = sbuf;
	}
	else {
		ptemp = apr_palloc(p, LOGALLOCSIZE(insize * 2));
	}
	
	if (pcon->m_iFromEncode == ENCODE_GB2312)
		usetable = gb2unicode;
	else
		usetable = big2unicode;
	
	num = ConvertToUnicode(usetable, FFFE, pins, insize, ptemp);
	
	num = unicode2utf8(ptemp, num, 1, pout);
	
	return num;
}

int AnsiToUTF8(char *pTable, char *pIns, int nSize, char *pPut)
{
	int nNum, nNext = sizeof(char) * 2;
	char *ptr, *pUse = pIns, *pOut = pPut, *pEnd = pIns + nSize;
	unsigned char chHigh, chLow;
	
	while (pUse < pEnd)
	{
		chHigh = (unsigned char) *pUse++;
		
		if (chHigh & 0x80)
		{
			if (pUse == pEnd)
			{
				*pOut++ = *(pUse - 1);
				break;
			}
			else
				chLow = (unsigned char) *pUse++;
			
			nNum = chHigh * 256 + chLow;
			ptr =  pTable + nNext * nNum;
			chHigh = (unsigned char) *ptr++;
			chLow = (unsigned char) *ptr;
			
			nNum = chHigh * 256 + chLow;
			
			/*
			if ((nNum & 0xFF00) == 0)
			*pOut++ = (char)nNum;
			else if ((nNum & 0xFC00) == 0)
			{
			*pOut++ = ((unsigned char)(nNum >> 6) & 0x1F) | 0xC0;
			*pOut++ = ((unsigned char)nNum & 0x3F) | 0x80;			
			}
			else
			{
			*pOut++ = ((unsigned char)(nNum >> 12) & 0xF) | 0xE0;
			*pOut++ = ((unsigned char)(nNum >> 6) & 0x3F) | 0x80;
			*pOut++ = ((unsigned char)nNum & 0x3F) | 0x80;
			}
			*/
			if (nNum < 0x80) {
				*pOut++ = nNum >> 0 & 0x7F | 0x00;
			}
			else if (nNum < 0x0800) {
				*pOut++ = nNum >> 6 & 0x1F | 0xC0;
				*pOut++ = nNum >> 0 & 0x3F | 0x80;
			}
			else if (nNum < 0x010000) {
				*pOut++ = nNum >> 12 & 0x0F | 0xE0;
				*pOut++ = nNum >> 6 & 0x3F | 0x80;
				*pOut++ = nNum >> 0 & 0x3F | 0x80;
			}
			else if (nNum <0x110000) {
				*pOut++ = nNum >> 18 & 0x07 | 0xF0;
				*pOut++ = nNum >> 12 & 0x3F | 0x80; 
				*pOut++ = nNum >> 6 & 0x3F | 0x80; 
				*pOut++ = nNum >> 0 & 0x3F | 0x80; 
			}
		}
		else
		{
			*pOut++ = (char)chHigh;
		}		
	}
	
	return pOut - pPut;
}

int ConvertToUTF8Ex(int fromencode, char *pins, int insize, char *pout, config *pcon)
{
	char *usetable;
	
	if (fromencode == ENCODE_GB2312)
		usetable = gb2unicode;
	else
		usetable = big2unicode;
	
	return AnsiToUTF8(usetable, pins, insize, pout);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int convert_orig_speed(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	int isword;
	unsigned int word, outsize=0;
	char buffwrite[8192], *bak = buffwrite, *pot = inbuf, *pend = inbuf + insize;
	rule *element;
	ruletable **wordlist;
	
	if (pcon->m_iFromEncode == pcon->m_iToEncode || pcon->m_iUnconvertOutput == 1)
	{
		memstream_write(pstream,inbuf,insize);
		return insize;
	}
	
	if (pcon->m_iFromEncode == ENCODE_GB2312)
		wordlist=wordgbbig;
	else
		wordlist=wordbiggb;
	
	if (pctx)	
		wordlist=pctx;
	
	while(pot < pend)
	{
		word=(unsigned char)*pot;
		
		if ((word & 0x80)==0)
			isword=0;
		else
		{
			unsigned char chi = *pot;
			unsigned char chw = *(pot+1);
			word = chi * 256 + chw;
			isword=1;
		}
		
		if (wordlist[word]!=NULL)
		{
			element=wordlist[word]->body;
			
			while (element!=NULL)
			{	
				if (pot+element->lenreal>inbuf+insize || memcmp(pot,element->realcode,element->lenreal))
				{
					element=element->link;
					continue;
				}
				break;
			}
			if (element!=NULL)
			{
				memcpy(bak,element->repcode,element->lenrep);
				bak+=element->lenrep;
				pot+=element->lenreal;
			}			
		}
		
		if ((wordlist[word]==NULL) || (element==NULL))
		{
			if (isword)
			{
				if (pcon->m_iFromEncode == ENCODE_GB2312)
				{
					*(bak++) = (char)gb2big5[(unsigned char)*pot][(unsigned char)*(pot+1)][0];
					*(bak++) = (char)gb2big5[(unsigned char)*pot][(unsigned char)*(pot+1)][1];
				}
				else
				{
					*(bak++) = (char)big52gb[(unsigned char)*pot][(unsigned char)*(pot+1)][0];
					*(bak++) = (char)big52gb[(unsigned char)*pot][(unsigned char)*(pot+1)][1];
				}
				
				pot+=2;
			}
			else
				*bak++=*pot++;
		}
		
		if	((bak - buffwrite) > 4000 || pot >= pend)
		{
			memstream_write(pstream,buffwrite,bak-buffwrite);
			outsize+=bak-buffwrite;
			bak=buffwrite;
		}
		
	}
	
	return(outsize);
}

int convert_orig_function_fjtignoreurl(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	int	  isword;
	unsigned int word,outsize=0;
	char  *bak,*pot,buffwrite[8192];	
	char *pend = NULL;	
	int convert_word = pcon->m_iConvertWord;
	int fromencode = pcon->m_iFromEncode;
	int toencode = pcon->m_iToEncode;
	int ishkword = pcon->m_iIshkword;		
	
	if (fromencode==toencode || (pcon->m_iUnconvertOutput == 1))
	{
		memstream_write(pstream,inbuf,insize);
		return insize;
	}
	
	/*
	WPF 2003-4-5
	CheckLeftBuffer(locaptr, pcon, &inbuf, &insize);
	*/ 
	pend = inbuf + insize;
	
	pot=inbuf;
	bak=buffwrite;
	
	while(pot < pend)
	{
		word = (unsigned char)*pot;		 
		
		if ((word & 0x80) == 0)
			isword = 0;
		else
		{
			if (pot + 1 == pend) {
				if (locaptr->nbakinfo < 1) {
					locaptr->bakinfo = apr_palloc(locaptr->p, LOGALLOCSIZE(4096));
					locaptr->nbakinfo = 4096;
				}
				locaptr->bakno = 1;
				locaptr->bakinfo[0] = *pot;
				break;
			}
			else {
				unsigned char chi = *pot;
				unsigned char chw = *(pot + 1);
				word = chi * 256 + chw;
				isword = 1;
			}
		}
		
		if (isword)
		{
			int num;
			char chbuff[128] = {*pot, *(pot + 1)};
			char chtemp[128] = {'\0'};
			
			// WPF 2002-7-24
			num = ConvertToUnicodeExt(pcon->m_iFromEncode, 1, FFFE, chbuff, 2, chtemp);
			
			if (num > 2)
			{
				memcpy(bak, chtemp, num);
				bak += num;
			}
			else
			{
				*bak++ = *pot;
				*bak++ = *(pot + 1);
			}
			pot+=2;
		}
		else
			*bak++=*pot++;
		
		if (bak - buffwrite > 4000) {
			memstream_write(pstream,buffwrite,bak-buffwrite);
			outsize+=bak-buffwrite;
			bak=buffwrite;
		}
	}
	
	{
		int left = bak - buffwrite;
		
		if	(left > 0) {
			memstream_write(pstream, buffwrite, left);
			outsize += left;			
		}
	}
	
	return(outsize);
}

int convert_orig_function(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	int	  i,k,isword;
	unsigned int word,outsize=0;
	char  *bak,*pot,buffwrite[8192];	
	char  *ptr,sub[RULELEN],str[RULELEN];
	rule  *element;
	ruletable **wordlist;
	
	char *pend = NULL;
	
	int convert_word = pcon->m_iConvertWord;
	int fromencode = pcon->m_iFromEncode;
	int toencode = pcon->m_iToEncode;
	int ishkword = pcon->m_iIshkword;		
	
	if (fromencode==toencode || (pcon->m_iUnconvertOutput == 1))
	{
		memstream_write(pstream,inbuf,insize);
		return insize;
	}
	
	if (pctx==NULL)
	{
		if ((fromencode==ENCODE_GB2312) && (toencode==ENCODE_BIG5))		
			wordlist=wordgbbig;
		if ((fromencode==ENCODE_BIG5) && (toencode==ENCODE_GB2312))		
			wordlist=wordbiggb;
	}
	else
		wordlist=pctx;
	
		/*
		WPF 2003-4-5
		CheckLeftBuffer(locaptr, pcon, &inbuf, &insize);
	*/ 
	pend = inbuf + insize;
	
	pot=inbuf;
	bak=buffwrite;
	
	while(pot < pend)
	{
		word = (unsigned char)*pot;		 
		
		if ((word & 0x80) == 0)
			isword = 0;
		else
		{
			if (pot + 1 == pend) {
				if (locaptr->nbakinfo < 1) {
					locaptr->bakinfo = apr_palloc(locaptr->p, LOGALLOCSIZE(4096));
					locaptr->nbakinfo = 4096;
				}
				locaptr->bakno = 1;
				locaptr->bakinfo[0] = *pot;
				break;
			}
			else {
				unsigned char chi = *pot;
				unsigned char chw = *(pot + 1);
				word = chi * 256 + chw;
				isword = 1;
			}
		}
		
		if (convert_word && (wordlist[word]!=NULL))		
		{
			element=wordlist[word]->body;
			
			if (!wordlist[word]->flag)
			{
				while (element!=NULL)
				{	
					if (pot+element->lenreal>inbuf+insize || memcmp(pot,element->realcode,element->lenreal))
					{
						element=element->link;
						continue;
					}
					break;
				}
				if (element!=NULL)
				{
					memcpy(bak,element->repcode,element->lenrep);
					bak+=element->lenrep;
					pot+=element->lenreal;
				}
			}
			else
			{
				while (element!=NULL)
				{	
					if (!element->flag)
					{
						if (pot+element->lenreal>inbuf+insize || memcmp(pot,element->realcode,element->lenreal))
						{
							element=element->link;
							continue;
						}
						memcpy(bak,element->repcode,element->lenrep);
						bak+=element->lenrep;
						pot+=element->lenreal;
						break;
					}
					else
					{
						for(i=0;(i<element->lenreal) && (pot+element->lenreal<=inbuf+insize);i++)
						{
							if ((*(pot+i)!=(element->realcode)[i]) && ((element->realcode)[i]!='#'))
							{
								element=element->link;
								break;
							}
						}
						if ((element!=NULL) && (i==element->lenreal))
						{
							if (!strchr(element->repcode,'#'))
							{
								memcpy(bak,element->repcode,element->lenrep);
								bak+=element->lenrep;
								pot+=element->lenreal;
								break;
							}
							else
							{
								ptr=element->realcode;
								for(i=0;i<element->lenreal;i++)
								{
									if (!strchr(ptr,'#'))
										break;
									ptr=strchr(ptr,'#');
									k=ptr-element->realcode;
									sub[i]=*(pot+k);
									ptr++;
								}
								k=i;
								strncpy(str,element->repcode,element->lenrep);
								ptr=str;								
								for(i=0;i<k;i++)
								{
									if (!strchr(ptr,'#'))
										break;
									ptr=strchr(ptr,'#');
									*ptr=sub[i];										
									ptr++;
								}
								memcpy(bak,str,element->lenrep);
								bak+=element->lenrep;
								pot+=element->lenreal;
								break;
							}
						}
					}
					
				}
			}
			
		}
		if (!convert_word || (wordlist[word]==NULL) || (element==NULL))
		{
			if (isword)
			{
				if ((fromencode==ENCODE_GB2312) && (toencode==ENCODE_BIG5))		
				{// WPF 2002-3-20
					unsigned char chl = gb2big5[(unsigned char)*pot][(unsigned char)*(pot+1)][0];
					unsigned char chr = gb2big5[(unsigned char)*pot][(unsigned char)*(pot+1)][1];
					
					if ((chl == 0xFF) && (chr == 0xFF))
					{
						unsigned char u1 = (unsigned char)*pot;
						unsigned char u2 = (unsigned char)*(pot+1);
						
						if ((u1 == 0xFF && u2 == 0xFE)
							|| (u1 == 0xFE && u2 == 0xFF))
						{
							// 0xFFFE, 0xFEFF skip 
						}
						else {
							int num;
							char chbuff[128] = {*pot, *(pot + 1)};
							char chtemp[128] = {'\0'};
							
							// WPF 2002-7-24
							num = ConvertToUnicodeExt(pcon->m_iFromEncode, 1, FFFE, chbuff, 2, chtemp);
							
							if (num > 2)
							{
								memcpy(bak, chtemp, num);
								bak += num;
							}
							else
							{
								*bak++ = chl;
								*bak++ = chr;
							}
						}
					}
					else
					{
						*bak++ = chl;
						*bak++ = chr;
					}
				}
				
				if ((fromencode==ENCODE_BIG5) && (toencode==ENCODE_GB2312))		
				{
					if (ishkword && (locaptr->inbody) &&
						(big52gb[(unsigned char)*pot][(unsigned char)*(pot+1)][0] == 255) &&
						(big52gb[(unsigned char)*pot][(unsigned char)*(pot+1)][1] == 255) &&
						(hktogb[(unsigned char)*pot][(unsigned char)*(pot+1)] != NULL))
					{
						int  n;
						char string[1024];
						
						if (locaptr->hkuse[hktogb[(unsigned char)*pot][(unsigned char)*(pot+1)]->group])
						{
							n = makeword(string, (unsigned char)*pot, (unsigned char)*(pot+1), hktogb[(unsigned char)*pot][(unsigned char)*(pot+1)]->group, locaptr->nIsNetscape);
							memcpy(bak, string, n);
							bak += n;
						}
						else
						{
							n = makefont(string, pcon->m_pcHkPrefix, hktogb[(unsigned char)*pot][(unsigned char)*(pot+1)]->group, locaptr);
							locaptr->hkuse[hktogb[(unsigned char)*pot][(unsigned char)*(pot+1)]->group] = 1;
							memcpy(bak, string, n);
							bak += n;
							n = makeword(string, (unsigned char)*pot, (unsigned char)*(pot+1), hktogb[(unsigned char)*pot][(unsigned char)*(pot+1)]->group, locaptr->nIsNetscape);
							memcpy(bak, string, n);
							bak += n;
						}
					}
					else
					{
						*(bak++)=(char)big52gb[(unsigned char)*pot][(unsigned char)*(pot+1)][0];
						*(bak++)=(char)big52gb[(unsigned char)*pot][(unsigned char)*(pot+1)][1];
					}
				}
				pot+=2;
			}
			else
				*bak++=*pot++;
		}
		
		if (bak - buffwrite > 4000) {
			memstream_write(pstream,buffwrite,bak-buffwrite);
			outsize+=bak-buffwrite;
			bak=buffwrite;
		}
	}
	
	{
		int left = bak - buffwrite;
		
		if	(left > 0) {
			memstream_write(pstream, buffwrite, left);
			outsize += left;			
		}
	}
	
	return(outsize);
}

int convert_orig(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	if (locaptr->fjtignoreurl == 1) {
		return convert_orig_function_fjtignoreurl(pctx, inbuf, insize, pstream, pcon, locaptr);
	}
	
	if (pcon->m_iAutoUseUnicode == 1
		|| pcon->m_iIshkword || !pcon->m_iConvertWord)
		return convert_orig_function(pctx, inbuf, insize, pstream, pcon, locaptr);
	else
		return convert_orig_speed(pctx, inbuf, insize, pstream, pcon, locaptr);
}

int convert(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	if (locaptr->iContentIsUnicode != 1)
		return convert_orig(pctx, inbuf, insize, pstream, pcon, locaptr);
	
	{//Process Unicode content
		int  from, to, num = 0;
		char *buff;
		
		buff = apr_palloc(locaptr->p, LOGALLOCSIZE(insize * 3 / 2 + 1024));
		
		if (pcon->m_iFromEncode == ENCODE_GB2312)
		{
			from = 	UNICODE_GB2312;
			to = UNICODE_BIG5;
		}
		else
		{
			from = UNICODE_BIG5;
			to = UNICODE_GB2312;
		}
		
		num = ConvertUnicodeExt(from, to, pcon->m_iConvertWord, inbuf, insize, buff);
		memstream_write(pstream, buff, num);		
		
		return num;		
	}
}

int unconvert_orig(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	int num;
	config excon;
	
	memcpy(&excon, pcon, sizeof(config));
	excon.m_iFromEncode = pcon->m_iToEncode;
	excon.m_iToEncode   = pcon->m_iFromEncode;
	excon.m_usetable    = pcon->m_negtable;
	excon.m_iUnconvertOutput = 0;
	
	if (excon.m_iForceOutputUTF8 == 1 && excon.m_iUseUnicodeTable == 1) {
		int  from, to;
		char *buff = NULL, *buff2 = NULL;
		
		if (excon.m_iFromEncode == ENCODE_GB2312) {
			from = 	UNICODE_GB2312;
			to = UNICODE_BIG5;
		}
		else {
			from = UNICODE_BIG5;
			to = UNICODE_GB2312;
		}
		buff = apr_palloc(locaptr->p, LOGALLOCSIZE(insize * 3 / 2 + 1024));        
		num = utf82unicode(inbuf, insize, buff, locaptr, FFFE);
		if (num > 0) {
			buff2 = apr_palloc(locaptr->p, LOGALLOCSIZE(insize * 3 / 2 + 1024));
			num = ConvertUnicodeExt(from, to, excon.m_iConvertWord, buff, num, buff2);
			if (num > 0) {
				num = ConvertFromUnicodeEx(pcon->m_iFromEncode, FFFE, buff2, num, buff);
				if (num > 0) {
					memstream_write(pstream, buff, num);
					return num;
				}
			}
		}        
	}
	
	{// WPF 2003-8-20
		if (locaptr->nUrlEncode == ENCODE_UNKNOW && excon.m_iForceOutputUTF8 == 1) {
			insize = ConvertFromUTF8(inbuf, insize, &excon, locaptr);            
		}
	}
	
	// if (locaptr->nIsUTF8 > 0 || excon.m_iSendURLsAsUTF8 == 1) {
	if (locaptr->nIsUTF8 > 0 || excon.m_iForcePostUTF8 == 1) {
		int bnum = 0;
		char buff[2048 * 3], temp[2048 * 3];
		char *pb = buff, *pt = temp;		
		memstream *tempmem = create_memstream(locaptr->p);
		
		num = convert(excon.m_usetable, inbuf, insize, tempmem, &excon, locaptr);		
		if (num > 2048)
		{
			pb = apr_palloc(locaptr->p, LOGALLOCSIZE(num * 3));
			pt = apr_palloc(locaptr->p, LOGALLOCSIZE(num * 3));
		}		
		num = memstream_read(tempmem, pt, num);
		
		bnum = num;
		num = ConvertToUTF8(pt, num, pb, pcon, locaptr->p);		
		memstream_write(pstream, pb, num);						
		if (bnum != num) {
			locaptr->nUrlEncode = ENCODE_UTF8;
		}
	}
	else {
		num = convert(excon.m_usetable, inbuf, insize, pstream, &excon, locaptr);
	}
	
	return num;
}

int unconvert(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr)
{
	int num;
	config excon;
	
	memcpy(&excon, pcon, sizeof(config));
	excon.m_iFromEncode = pcon->m_iToEncode;
	excon.m_iToEncode   = pcon->m_iFromEncode;
	excon.m_usetable    = pcon->m_negtable;
	excon.m_iUnconvertOutput = 0;
	
	if (excon.m_iPostDataIsUTF8 == 1)
	{// WPF 2005-9-29 9:29
		char buff[2048 * 3], temp[2048 * 3];
		char *pb = buff, *pt = temp;		
		
		memstream *tempmem = create_memstream(locaptr->p);        
		insize = ConvertFromUTF8(inbuf, insize, &excon, locaptr);            
		num = convert(excon.m_usetable, inbuf, insize, tempmem, &excon, locaptr);        
		if (num > 2048) {
			pb = apr_palloc(locaptr->p, LOGALLOCSIZE(num * 3));
			pt = apr_palloc(locaptr->p, LOGALLOCSIZE(num * 3));
		}  
		num = memstream_read(tempmem, pt, num);
		num = ConvertToUTF8(pt, num, pb, pcon, locaptr->p);        
		memstream_write(pstream, pb, num);
		
		return num;
	}
	
	if (excon.m_iForceOutputUTF8 == 1 && excon.m_iUseUnicodeTable == 1) {
		int  from, to;
		char *buff = NULL, *buff2 = NULL;
		
		if (excon.m_iFromEncode == ENCODE_GB2312) {
			from = 	UNICODE_GB2312;
			to = UNICODE_BIG5;
		}
		else {
			from = UNICODE_BIG5;
			to = UNICODE_GB2312;
		}
		buff = apr_palloc(locaptr->p, LOGALLOCSIZE(insize * 3 / 2 + 1024));        
		num = utf82unicode(inbuf, insize, buff, locaptr, FFFE);
		if (num > 0) {
			buff2 = apr_palloc(locaptr->p, LOGALLOCSIZE(insize * 3 / 2 + 1024));
			num = ConvertUnicodeExt(from, to, excon.m_iConvertWord, buff, num, buff2);
			if (num > 0) {
				num = ConvertFromUnicodeEx(pcon->m_iFromEncode, FFFE, buff2, num, buff);
				if (num > 0) {
					memstream_write(pstream, buff, num);
					return num;
				}
			}
		}        
	}
	
	{// WPF 2003-8-20
		if (excon.m_iForceOutputUTF8 == 1) {
			insize = ConvertFromUTF8(inbuf, insize, &excon, locaptr);            
		}
	}
	
	if (excon.m_iForcePostUTF8 == 1 || locaptr->nIsUTF8 > 0) { // || excon.m_iSendURLsAsUTF8 == 1) {
		// if (locaptr->nIsUTF8 > 0) {
		char buff[2048 * 3], temp[2048 * 3];
		char *pb = buff, *pt = temp;		
		memstream *tempmem = create_memstream(locaptr->p);
		
		num = convert(excon.m_usetable, inbuf, insize, tempmem, &excon, locaptr);
		
		if (num > 2048)
		{
			pb = apr_palloc(locaptr->p, LOGALLOCSIZE(num * 3));
			pt = apr_palloc(locaptr->p, LOGALLOCSIZE(num * 3));
		}
		
		num = memstream_read(tempmem, pt, num);
		
		num = ConvertToUTF8(pt, num, pb, pcon, locaptr->p);
		
		memstream_write(pstream, pb, num);		
	}
	else {
		num = convert(excon.m_usetable, inbuf, insize, pstream, &excon, locaptr);
	}
	
	return num;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int InitOnlyUnicode(pool *p)
{
	// int heapsize = 4096 * 400;
	int heapsize = sizeof(ruletable) * FJTMAXWORD + sizeof(rule) * 8192;
	int buffsize = 256 * 256 * 2;
	
	UniGbUniBig = (unsigned char*) apr_palloc(p, buffsize);
	UniBigUniGb = (unsigned char*) apr_palloc(p, buffsize);
	UniGbUniBigRule = (ruletable*) apr_palloc(p, heapsize);
	UniBigUniGbRule = (ruletable*) apr_palloc(p, heapsize);
	
	memset(UniGbUniBig, 0, buffsize);
	memset(UniBigUniGb, 0, buffsize);
	memset(UniGbUniBigRule, 0, heapsize);
	memset(UniBigUniGbRule, 0, heapsize);
	
#ifdef WIN32
	{/* Share Unicode word table */		
		getdir(dir,sizeof(dir));
		strcat(dir,"etc\\ungb2bigword.txt");
		if (!InitUnicodeWord(UniGbUniBig, dir))
			return 0;		
		getdir(dir,sizeof(dir));
		strcat(dir,"etc\\unbig2gbword.txt");
		if (!InitUnicodeWord(UniBigUniGb, dir))
			return 0;
	}
	
	{/* Share Unicode rule table */		
		getdir(dir, sizeof(dir));
		strcat(dir, "etc\\ungb2bigrule.txt");		
		{
			char *ptr = NULL;
			if (!InitUnicodeRule(UniGbUniBigRule, ptr, dir))				
				return 0;
		}
		getdir(dir, sizeof(dir));
		strcat(dir, "etc\\unbig2gbrule.txt");		
		{
			char *ptr = NULL;
			if (!InitUnicodeRule(UniBigUniGbRule, ptr, dir))
				return 0;
		} 
	}	
#else
	{/* Share Unicode word table */
		{
			char buf[512];
			getdir(buf, sizeof(buf));
			strcat(buf, "etc/ungb2bigword.txt");
			if (!InitUnicodeWord(UniGbUniBig, buf))
				return 0;
			getdir(buf, sizeof(buf));
			strcat(buf, "etc/unbig2gbword.txt");
			if (!InitUnicodeWord(UniBigUniGb, buf))
				return 0;
		}
	}
	
	{/* Share Unicode rule table */
		{
			char *ptr = NULL;
			char buf[512];
			getdir(buf, sizeof(buf));
			strcat(buf, "etc/ungb2bigrule.txt");
			if (!InitUnicodeRule(UniGbUniBigRule, ptr, buf))				
				return 0;
		}				
		{
			char *ptr = NULL;
			char buf[260];
			getdir(buf, sizeof(buf));
			strcat(buf, "etc/unbig2gbrule.txt");
			if (!InitUnicodeRule(UniBigUniGbRule, ptr, buf))
				return 0;
		}
	}		
#endif
	
	return 1;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int init_convert(pool *p)
{
	char buf[512];
	
	buf[0] = '\0';
	/* Init table once */
	if (++init_count > 1)
		return 1;
	
	/* Use for get copyright */
	forcopyright();
	
	if (!InitConverttable())
		return 0;
	
#ifdef WIN32	
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\gb2big5.cvt2");
	if (!initfile(wordgbbig, NULL, dir, p))
		return(0);
	
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\big52gb.cvt1");
	if (!initfile(wordbiggb, NULL, dir, p))
		return(0);
	
	getdir(dir,sizeof(dir));
	strcat(dir,"etc\\hk2gb.txt");
	if (!inithkword(hktogb,dir, p))
		return(0);
#else
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/gb2big5.cvt2");
	if (!initfile(wordgbbig, NULL, buf, p))		
		return(0);
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/big52gb.cvt1");
	if (!initfile(wordbiggb, NULL, buf, p))	
		return(0);
	getdir(buf, sizeof(buf));
	strcat(buf, "etc/hk2gb.txt");
	if (!inithkword(hktogb, buf, p))	
		return(0);
#endif
	
	/* initunicode_no2(...) use for Change betwen ANSI and UNICODE (UTF-8) */
	// InitOnlyUnicode(...) Only use for Unicode convert
	if (!initunicode() || !initunicode_no2(p) || !InitOnlyUnicode(p))
		return(0);
	
	return(1);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int ConvertVar(void *info, int from, int to, int conword, char *pins, int insize, char *pout)
{
	unsigned int word, isword;
	char *bak, *pot;	
	rule *element;
	ruletable *wordlist, *pBackup;
	
	char *pUse;
	int  nStart, nHead, nOffset;
	
	static int high = 256*2, low = 2;
	unsigned char *gb2big5, *big52gb;	
	ruletable *wordgbbig, *wordbiggb;	
	LPINPUTINFO ptrinfo = (LPINPUTINFO)info;
	
	if (ptrinfo == NULL)
		return 0;
	
	gb2big5 = ptrinfo->gb2big5;
	big52gb = ptrinfo->big52gb;
	wordgbbig = ptrinfo->wordgbbig;
	wordbiggb = ptrinfo->wordbiggb;
	
	if (!gb2big5 || !big52gb || !wordgbbig || !wordbiggb)
		return 0;
	
	if ((from == BIG5) && (to == GB2312))
		pBackup = wordbiggb;
	else if ((from == GB2312) && (to == BIG5))
		pBackup = wordgbbig;
	else
	{
		memcpy(pout, pins, insize);
		return insize;
	}
	
	pot = pins;
	bak = pout;
	nStart = pBackup->flag;	
	nHead = (int)pBackup;
	
	while(pot < pins+insize)
	{
		word = (unsigned char)*pot;		 
		
		if ((word & 0x80) == 0 || pot+1 == pins+insize )
			isword = 0;
		else
		{
			unsigned char chi = *pot;
			unsigned char chw = *(pot+1);
			word = chi * 256 + chw;
			isword = 1;
		}
		
		pUse = (char*)pBackup;		
		pUse += sizeof(ruletable)*word;
		wordlist = (ruletable*)pUse;
		
		if (conword && (wordlist->body != NULL))		
		{
			nOffset = (int)(wordlist->body) - nStart;
			
			element = (rule*)(nHead + nOffset);
			
			while (element != NULL)
			{
				if (pot+element->lenreal > pins+insize || memcmp(pot,element->realcode,element->lenreal))
				{
					if (element->link == NULL)
					{
						element = element->link;
						break;
					}
					nOffset = (int)(element->link) - nStart;
					element = (rule*)(nHead + nOffset);
					continue;
				}
				break;
			}
			if (element != NULL)
			{
				memcpy(bak,element->repcode,element->lenrep);
				bak += element->lenrep;
				pot += element->lenreal;
			}
		}
		
		if (!conword || (wordlist->body == NULL) || (element == NULL))
		{
			if (isword)
			{
				if ((from == GB2312) && (to == BIG5))		
				{
					*(bak++)=(char)*(gb2big5 + ((unsigned char)*pot)*high + ((unsigned char)*(pot+1))*low);
					*(bak++)=(char)*(gb2big5 + ((unsigned char)*pot)*high + ((unsigned char)*(pot+1))*low + 1);
				}
				if ((from == BIG5) && (to == GB2312))		
				{										
					*(bak++)=(char)*(big52gb + ((unsigned char)*pot)*high + ((unsigned char)*(pot+1))*low);
					*(bak++)=(char)*(big52gb + ((unsigned char)*pot)*high + ((unsigned char)*(pot+1))*low + 1);
				}
				pot+=2;
			}
			else
				*bak++=*pot++;
		}		
	}
	
	return(bak-pout);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int ConvertUnicode(void *info, int from, int to, int conword, char *pins, int insize, char *pout)
{
	unsigned int word;
	char *bak, *pot;	
	rule *element;
	ruletable *wordlist, *pBackup;
	
	char *pUse;
	int  nStart, nHead, nOffset;
	
	static int nh = 256*2, nl = 2;
	unsigned char *gb2big5, *big52gb;	
	ruletable *wordgbbig, *wordbiggb;	
	LPINPUTINFO ptrinfo = (LPINPUTINFO)info;
	
	if (ptrinfo == NULL)
		return 0;
	
	gb2big5 = ptrinfo->unigbbig;
	big52gb = ptrinfo->unibiggb;
	wordgbbig = ptrinfo->gbrule;
	wordbiggb = ptrinfo->bigrule;
	
	if (!gb2big5 || !big52gb || !wordgbbig || !wordbiggb)
		return 0;
	
	if ((from == UNICODE_BIG5) && (to == UNICODE_GB2312))
		pBackup = wordbiggb;
	else if ((from == UNICODE_GB2312) && (to == UNICODE_BIG5))
		pBackup = wordgbbig;
	else
	{
		memcpy(pout, pins, insize);
		return insize;
	}
	
	pot = pins;
	bak = pout;
	nStart = pBackup->flag;	
	nHead = (int)pBackup;
	
	while(pot < pins+insize)
	{
		word = (unsigned char)(*pot) * 256 + (unsigned char)*(pot + 1);
		
		pUse = (char*)pBackup;		
		pUse += sizeof(ruletable)*word;
		wordlist = (ruletable*)pUse;
		
		if (conword && (wordlist->body != NULL))		
		{
			nOffset = (int)(wordlist->body) - nStart;
			//实际上wordlist->body == element, 因为nStart == nHead,wordlist->body - nStart + nHead == wordlist->body
			element = (rule*)(nHead + nOffset);
			
			while (element != NULL)
			{

				if (pot+element->lenreal > pins+insize || memcmp(pot,element->realcode,element->lenreal))
				{
					//不匹配，则用下一个rule
					if (element->link == NULL)
					{
						element = element->link;
						break;
					}
					//相当于element == element->link
					nOffset = (int)(element->link) - nStart;
					element = (rule*)(nHead + nOffset);
					continue;
				}
				break;
			}
			//匹配了，转换
			if (element != NULL)
			{
				memcpy(bak,element->repcode,element->lenrep);
				bak += element->lenrep;
				pot += element->lenreal;
			}
		}
		
		if (!conword || (wordlist->body == NULL) || (element == NULL))
		{	
			unsigned char low, high;
			
			if ((from == UNICODE_GB2312) && (to == UNICODE_BIG5))		
			{				
				low = *(gb2big5 + (unsigned char)*pot * nh + (unsigned char)*(pot+1) * nl);			
				high = *(gb2big5 + (unsigned char)*pot * nh + (unsigned char)*(pot+1) * nl + 1);
			}
			if ((from == UNICODE_BIG5) && (to == UNICODE_GB2312))		
			{										
				low = *(big52gb + (unsigned char)*pot * nh + (unsigned char)*(pot+1) * nl);
				high = *(big52gb + (unsigned char)*pot * nh + (unsigned char)*(pot+1) * nl + 1);
			}
			
			if ((low == 255 && high == 255) || (low == 0 && high == 0))
			{
				*bak++ = *pot++;
				*bak++ = *pot++;
			}
			else
			{
				*bak++ = (char)low;
				*bak++ = (char)high;
				pot += 2;
			}
		}
	}
	
	return(bak-pout);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int ConvertUnicodeExt(int from, int to, int conword, char *pins, int insize, char *pout)
{
	INPUTINFO info;
	
	info.unigbbig = UniGbUniBig;
	info.unibiggb = UniBigUniGb;
	info.gbrule   = UniGbUniBigRule;
	info.bigrule  = UniBigUniGbRule;
	
	return ConvertUnicode(&info, from, to, conword, pins, insize, pout);
}

/*-----------------------------------------------------------------------------------------------------------------------*/