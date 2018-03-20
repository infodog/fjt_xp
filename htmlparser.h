#ifndef _HTMLPARSER_H_
#define _HTMLPARSER_H_

/*
#include "mm.h"
*/

#include "config.h"
#include "memstream.h"
#include "mod_proxy.h"

#ifndef WIN32
int MY_SYSTEM(char *command);
#endif 

int ProcessUnicode(config *pcon, ConvertCtx *pctx, char *pins, int nsize);
int ChangeScript(pool *apool, ConvertCtx *pctx, memstream *apwrite, config *pconf, char *pbuf, int nsize);
int ReplaceHTTPAndConvert(ConvertCtx *pctx, memstream *apwrite, config *pconf, char *pbuf, int nsize);
int ProcessScript(pool *apool, ConvertCtx *apCtx, config *pconf, memstream *apwrite,char *pbuf, int nsize);
int ParseHtml(pool *apool, ConvertCtx *apCtx, config *pconf, char *inbuf, int size, memstream *apwrite);
int ParseHtml_html(pool *apool,ConvertCtx *apCtx,config *pconf,char *inbuff,int sizee,memstream *apwrite);
int IsDomainValid(table *tb, char *host);
int DetermineFileType(proxy_server_conf *conf, config *pconfig, char *datestr, char *url);
int NeedProcess(char *pbuff, int len, config *pcon, ConvertCtx *pctx);
char* ap_proxy_cookie_domain(pool *pool, ConvertCtx *pctx, config *pconfig, char *cookies);
char* ResetUrl7E(char *url);
char* filter_out(int *insize, char *inbuff, config *pcon, ConvertCtx *pctx, int ispic);
char* get_in_buff(fjtconf *fc, int length);
char* get_out_buff(fjtconf *fc, int length);
char* filter_out_ex(fjtconf *fc, config *pcon, ConvertCtx *pctx, apr_pool_t *p, memstream *ms, apr_size_t *length);
int ProcessEnt(ConvertCtx *pctx, config *pconfig, memstream *pstream, char *pboundary, char *pentb, char *pente);
int GetHost(fjtconf *fc);
int ProcessMETA(int ToEncode, char *inbuf, int len, char *outbuf);
void RedirectTip(request_rec *r, char *url, char *tips, config *pconfig);
int ProcessTag(pool *apool, ConvertCtx *apCtx, config *pconf, char *ptagb, int nsize, memstream *apwrite);
int DetectUTF8BOM(char *data, int len, int strict);
char* ProcessCookies(pool *p, char *cookie, char *url, config *pconfig, request_rec  *r);
int From3A2F(char *pin, int nsize, char *pout);
int To3A2F(char *pin, int nsize, char *pout);
int ResetUrl(char *pin, int nsize, char *pck, int nck);
void contenttype(config *pconfig, ConvertCtx *pctx, int file_type, char *datestr, char *cntype);
apr_size_t copy_brigade_to_memstream(apr_bucket_brigade *bb, 
									 memstream *ms) ;
apr_status_t my_pass_memstream(fjtconf *fc, memstream *ms, 
							   request_rec *r, proxy_server_conf *conf, ap_filter_t *filter);
void prepare_pdata_length(config *pconfig, ConvertCtx *pctx, char **pdata, int *length);
apr_status_t my_pass_brigade(fjtconf *fc, request_rec *r, proxy_server_conf *conf,
							 ap_filter_t *filter, apr_bucket_brigade *bucket);
apr_status_t my_flush_session(fjtconf *fc, request_rec *r, proxy_server_conf *conf, ap_filter_t *filter);
config* FindConfig(config *pcfg, char *url, request_rec *r, proxy_server_conf *conf);
int check_infoscape(request_rec *r, apr_pool_t *p, char *url);
void init_session(fjtconf *session, request_rec *r, apr_pool_t *p);
void clean_session(fjtconf *session);
int CheckDomain(proxy_server_conf *conf, request_rec *r, apr_pool_t *p, fjtconf *fc, char *url);

#endif