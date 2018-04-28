#ifndef FJT_CONFIG_H_
#define FJT_CONFIG_H_

#include "fjt.h"

#include "apr_hooks.h"
#include "apr.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_buckets.h"
#include "apr_md5.h"
#include "apr_network_io.h"
#include "apr_pools.h"
#include "apr_strings.h"
#include "apr_uri.h"
#include "apr_date.h"
#include "apr_fnmatch.h"
#define APR_WANT_STRFUNC
#include "apr_want.h"
#include "httpd.h"
#include "http_config.h"
#include "ap_config.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_vhost.h"
#include "http_main.h"
#include "http_log.h"
#include "http_connection.h"
#include "util_filter.h"
#include "util_ebcdic.h"
#include "apr_tables.h"

typedef apr_pool_t pool;
typedef apr_table_t table;
typedef apr_table_entry_t table_entry;
typedef apr_array_header_t array_header;

#define ENCODE_GB2312 0
#define ENCODE_BIG5 1
#define ENCODE_UTF8 2
#define ENCODE_UNKNOW 3
/* // For ConvertExt Type -- */
#define GB2312 0
#define BIG5 1
#define UNICODE_GB2312 2
#define UNICODE_BIG5 3
#define UTF8_GB2312 4
#define UTF8_BIG5 5
#define UTF8 6
#define FFFE 0xFFFE
#define FEFF 0xFEFF
/* //----------------------- */
#define FILETYPE_AUTO_DETECT 0
#define FILETYPE_JS 1
#define FILETYPE_HTML 2
#define FILETYPE_TEXT 3
#define FILETYPE_PLAINHTML 4
/* //----------------------- */
#define URL_DELIMITER "@OldUrl;"
#define URL_TYPE_JS "@JS;"
#define URL_TYPE_IMG "@IMG;"
#define URL_TYPE_BASE "@BASE;"
#ifndef FJTMAXWORD
#define FJTMAXWORD 65536
#endif
#define RULELEN 128

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

int BLOCKLEVELSIZELOG(int size, char *file, int line);
/* // #define LOGALLOCSIZE(val) BLOCKLEVELSIZELOG(val, __FILE__, __LINE__)
// #define BLOCKLEVELSIZE(n) (((n) + 7) & ~7) */
#define LOGALLOCSIZE(n) (n)

#include "convert_utils.h"



/* // WPF */
typedef struct hosts_addr_domain {
    char *ipstr;
    apr_port_t ipstr_port;
    char *domain;
    apr_port_t domain_port;
    struct hosts_addr_domain *next;
} hosts_addr_domain;
/* // WPF */

typedef struct config
{
	char *location; //the location of the config;

	/* the following are for HTML Parser */
	char *m_pcBaseUrl;
	int m_iFromEncode;
	int m_iToEncode;
	int m_iShouldExpandJs;
	char *m_pcUrlPrefix;
	char *m_pcSUrlPrefix; /*https ��url prefix*/
	int m_iShouldChangeUrlInServer;
	char *m_pcCookieDomain;
	int m_iIsRedirectAbsolute;
	int m_iConvertWord;
	ruletable **m_usetable;
	ruletable **m_negtable;
	int m_iIsUnicode;
	int m_iIsUTF8;
	int m_iDetectUTF8;
	int m_iDetectUTF8BOM;
	int m_iUnconvertOutput;
	int m_iNotChangeTextboxUrl;
	int m_iChangedUrlEncode;
	int m_iWebPageMaxSize;
	int m_iNotSetCookie;
	int m_iResumeLogs;
	int m_iAutoUseUnicode;
	int m_iProcessPicture;
	int m_iPostdataResetUrl;
	int m_iForceOutputUnicode;
	int m_iForceOutputUTF8;
	int m_iForcePostUTF8;
	int m_iNotConvertPage;
	int m_iUseOrigProxy;
	int m_iNotProcessPercent25;
	int m_iProcessWord;
	int m_iNotConvertGetPostData;
	int m_iNotModifyContentType;
	int m_iIgnoreUrlPrefix;
	int m_iUseUnicodeTable;
	int m_iOutputSameEncode;
	int m_iKeepUTF8Encode;
	int m_iTreatScriptAsHtml;
	int m_iPostDataIsUTF8;
	int m_iEscapeUTF8Char;
	int m_iIgnorePostDataEncode;
	int m_iNotDetectAttachment;
	int m_iPostRawDataAsUTF8;
	int m_iUnescapePostData;
	int m_iHandleOLDURL;

	int m_iUnescapeChar;
	int m_iOutputEscapeChar;
	int m_iMarkUTF8Url;
	int m_iNoMimeNoConvert;
	int m_iNotSetContentTypeCharset;
	int m_iIgnoreUrlEncode;
	int m_iUnConvertWholeUrl;
	int m_iRedirectTip;
	int m_iRedirectTipTime;
	char *m_pcRedirectTipString;
	char *m_pcRedirectTipURL;
	int m_iForceRedirectTip;
	int m_iForceConvertPage;
	int m_iSendURLsAsUTF8;
	int m_iNotReplaceUrl;
	int m_iConvertCookie;
	int m_iConvertContentDisposition;
	int m_iMergeCookie;
	int m_iAddUrlPrefixToParameter;

	int m_iNoCache;
	int m_iSeekPhoto;
	int m_iGetClientIP;
	int m_iIshkword;
	char *m_pcHkPrefix;
	char *m_pcInsertCSS;
	char *m_pcConfBaseUrl;

	char *m_pcExtraData;
	int m_iExtraDataIgnoreJs;
	char *m_pcJsPrefix;
	char *m_pcFramePrefix;
	char *m_pcHTTPPrefix;
	char *m_pcPath;
	int m_i_pcPath;
	int m_iInShouldConvert;
	int m_iInConvertWord;
	char *m_pcExtraCookie;
	char *m_pcUnconvertSymbol;
	char *m_pcSetContentType;
	char *m_pcNativeDomain;
	int m_iChangeChineseLevel;
	int m_iCookiePathMode; /*0 --- ȫ����'/', 1 --- ��/111/222/gb/www.21cn.com/��ģʽ*/

	int m_iProcessScriptLevel; /*0 --- ֻ��ReplaceHttp, 1 --- �����? URL_DELIMITER ��־,  2---���Ӵ����ķ���Script�ڲ�������*/
	int m_iAddTrailingSlash;   /*0 --- ���ڲ����� '/'��β��Ŀ¼����'/' 1---���ڲ����� '/'��β��Ŀ¼��'/' */
	int m_iReplaceHttpInText;  /*0 --- �������в��޸�HTTP, 1---���������޸�HTTP*/
	int m_iScriptChangeChineseLevel;
	int m_iChangeScriptByDefault;
	int m_iValueChangeChineseLevel;

	int m_iInConvertUnicode;

	table *m_pUrlMapJS;
	table *m_pUrlMapNE;
	table *m_pUrlModify;
	table *m_pUrlMap;
	table *m_pUseTableFile;
	struct config *m_pnext;
	struct config *m_pprev;

} config;



typedef struct svr_config
{
	/*  // WPF */
	time_t timeCheckPoint; /* // check license time limited. <any>2007 1 1 2007 4 5 */
	int nYesDomain;
	table *allowed_domain;
	int nExcludeDomain;
	table *exclude_domain;
	int nFriendlyDomain;
	table *friendly_domain;

	/* // WPF */
	int nAddrDomain;
	hosts_addr_domain *addr_domain;
	int m_iHostAddrPort;
	int m_iHostAddrPort_set;
	/*  // WPF */

	config *pconfig;
	int WebPageMaxSize;
	char WebPageMaxSize_set;
	int m_iExportApi;
	char m_iExportApi_set;
	char *m_pcBinaryFileExt[100];
	int m_iBinaryFileExt;
	char *m_pcHtmlFileExt[100];
	int m_iHtmlFileExt;
	char *m_pcKeepUrlSuffix[100];
	char *m_pcServerUrlPrefix[512];
	int m_iKeepUrlSuffix;
} svr_config;

typedef struct ConvertCtx
{
	pool *p;
	int m_istate;
	char *m_pcCurrentUrl;

	int bakno;
	char *bakinfo;
	int nbakinfo;
	char *p_BakBuf;
	int n_BakBuf;

	int unicodeno;
	char unicodeinfo[10];
	int nbakunicode;
	char *pchbakunicode;

	int nHexChar;
	char chHexChar[10];

	int nUnicodeFirst;
	int nUnicodeFormat;
	char cUnicodeLeft[10];

	int utf8no;
	char utf8info[10];

	int fjtignoreurl;
	int inbody;
	int hkuse[100];
	int nIsNetscape;
	int nAddPlugin;

	int nUrlEncode;
	int nIsUTF8;
	int iContentIsUnicode;
	int nOutput;
	char *pcOutput;
	int nNotChangeTextboxUrl;
	request_rec *r;
	svr_config *svr_conf;
	config *dir_conf;
	int nIsWord;
    int orig_out_charset;
	int prefix_len;
	int nBuffArraySize;
	char *pBuffArray[5];

} ConvertCtx;

typedef struct fjtconf
{
	config *pconfig;
	ConvertCtx pctx;
	char *url;
	int nFileType;
	char *host;
	int nport;
	int nflag;
	char *oldurl;
	int norder;
	pool *p;
	/* // WPF new ...  */
	apr_size_t aprstTotal;
	FILE *fpBody;
	char *cmd;
	char *fname;
	char *pfname;
	struct memstream *msBody;
	struct memstream *msBuff;
	struct memstream *ms_out;

	int i_in_size;
	char *pc_in_buff;
	int i_out_size;
	char *pc_out_buff;
	void *tmpbb;
	int orig_out_charset;
	int processed; //是否已经处理过，免得多次处理，浪费CPU
} fjtconf;

typedef struct hkword
{
	int group;
	char unicode[10];
} hkword;

void getdir(char *str,int size);
#endif
