#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "config.h"
#include "memstream.h"

#ifdef WIN32
void getdir(char *dir,int len);
#endif

typedef struct unicode_stream{
    int len;
    char buf[2048];
    FILE *fp;
    int isEnd;
} unicode_stream;

int init_stream(FILE *fp,unicode_stream *stream);
int readLineW(char *buf,int nbuf,unicode_stream *stream);

int  init_convert(pool *p);
int initfile(ruletable *wordlist[], ruletable *reverselist[], char *filename, pool *p,int isUtf16);
int InitOnlyUnicode(pool *p);
int  convert(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr);
int unconvert_orig(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr);

int  unconvert(ruletable** pctx,char *inbuf,int insize,memstream *pstream,config *pcon, ConvertCtx *locaptr);
void changeunicode(char *inbuff,int *insize,ConvertCtx *apCtx,int isunicode,int basecode);
int  utf82unicode(char *pinput, int insize, char *pout, ConvertCtx *apCtx, int twobytes);
int unicode2utf8(char *pins, int nsize, int exchange, char *pout);
char* ConvertFromUTF8_auto_unicode(char *pins, int insize, config *pcon, ConvertCtx *pctx, int *outSize);
int  ConvertFromUTF8(char *pins, int insize, config *pcon, ConvertCtx *pctx);
int  ConvertFromUTF8Ex(char *pins, int insize, int FromEncode, ConvertCtx *pctx);
int  ConvertFromUnicodeEx(int fromcoding, int flag, char *pIns, int nSize, char *pPut);
int  ConvertToUnicodeExt(int fromcode, int flag, int format, char *pIns, int nSize, char *pPut);
int ConvertToEscapeUnicode(int fromcode, int flag, int format, char *pIns, int nSize, char *pPut);
int  ConvertToUTF8Ex(int fromencode, char *pins, int insize, char *pout, config *pcon);
int hz_unescape(char *inbuf, int *size, int encode);
int my_unescape_ex(char **p2inbuf, int *size, config *pconfig, ConvertCtx *apCtx, int encode);
int  my_unescape(char *inbuf, int *size, ConvertCtx *apCtx, int encode);
int  my_escape(int fromcode, char *pin, int nsize, char *pout);
void CheckLeftBuffer(ConvertCtx *pctx, config *pconf, char **inbuffer, int *insize);
char* ismine(request_rec *r, char *url, pool *p, int *flag);
int ConvertUnicodeExt(int from, int to, int conword, char *pins, int insize, char *pout, int outsize);
int url_unicode2utf8(char *url, char *newurl);

int convert_lite_memstream(ruletable** ruletables, char *wordTable, char *inbuf, int insize,memstream *pstream,int isUtf16);
int convert_lite_buf(ruletable** ruletables, char *wordTable, char *inbuf, int insize,unsigned char *pbuf,int nbuf,int isUtf16);

int translate(int inOut,config *dc,char *inbuf, int insize,memstream *outStream,pool *pool);
#endif 
