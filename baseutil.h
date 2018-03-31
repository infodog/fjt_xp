#ifndef _BASEUTIL_H_
#define _BASEUTIL_H_

#include "config.h"

#ifdef USE_INTERNAL_MM
#include "mm.h"
#endif

#define SINGLE_QUOTE		1000
#define	DOUBLE_QUOTE		1001

#ifndef min
#define min(x, y)	(( x < y ) ? (x) : (y))
#define max(x, y)	(( x > y ) ? (x) : (y))
#endif

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif


void exception_handler(int sig);
void InitUrlEncodeTable(void);
void InitSafeTable(void);
int ChangeUrl_Lite(pool *apool, ConvertCtx *apCtx, config *pconf, char *apurl, int nsize, char **pNewurl, int *nNewUrl);
char* UnChangeChinese(pool *apool, config* pconfig, ConvertCtx *apCtx, char *apurl);
char* ChangeChinese(pool *apool, ConvertCtx *apCtx, config *pconf, char *apurl, int nsize);
int UrlDecode(config* pconfig, char *inbuf, int insize, char *outbuf, int *outsize);
int DataEncode(char *inbuf, int insize, char *outbuf, int *outsize);
int DataDecode(config* pconfig, char *inbuf, int insize, char *outbuf, int *outsize);
int UrlRepair(char *inbuf, int nsize);
int UrlEncode(char *inbuf, int insize, char *outbuf, int *outsize);
int ChangeUrl(pool *apool, ConvertCtx *apCtx, config* pconf, char *apurl, int nsize, char **pNewUrl, int *nNewUrl);
char* GetPathOfAbsUrl(pool *apool, char *pabsurl);
char* strnchr(char *s, int nsize, int c);
char* ConvertUrl(ConvertCtx *pctx, pool *apool, config *pconfig, char *url);
char* UnConvertUrl(ConvertCtx *pctx, pool *apool, config *pconfig, char *url);
char* strnistr(char *s1, char *s2, int n);
char* FindMatch(char *pbuf, int nbuf, int c);
char *BuildBase64Block(ConvertCtx *apCtx, char *srcStr,int nsize);
int UnBase64Block(char *inbuf, int size);
int ResetCode7E(char *url, int len);
char* UrlUnMap(config *pconf, pool *apool, char *pabsurl);
char* url_reset_prefix(char *u, char *s, char *d, pool *p);
char* url_http_proxy(char *url, char *http, char *proxy, pool *p);
char* trimstr(char *str);
int UrlDecodeHZ(char *inbuf, int insize, char *outbuf, int *outsize);
int UrlEncodeHZ(char *inbuf, int insize, char *outbuf);

#ifndef WIN32
void itoa(int num,char *str,int base);
#endif
#endif
