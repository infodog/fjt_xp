/* OS headers */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#ifndef  WIN32
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <windows.h>
#ifndef   _WIN32_WINNT   
#define   _WIN32_WINNT   0x0400   
#endif
#include <wincrypt.h>
#endif

#include "ifconfigip.h"

#include "baseutil.h"

/* OpenSSL headers */
#ifdef   SSL_EXPERIMENTAL
#ifdef   SSL_ENGINE
#ifndef  SSL_EXPERIMENTAL_ENGINE_IGNORE
#define  SSL_EXPERIMENTAL_ENGINE
#endif
#endif
#endif   /* SSL_EXPERIMENTAL */
#define  MOD_SSL_VERSION AP_SERVER_BASEREVISION
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/ocsp.h>
#ifdef   SSL_EXPERIMENTAL_ENGINE
#include "engine.h"
#endif
#ifdef   HAVE_SSL_X509V3_H
#include "x509v3.h"
#endif

/* Apache headers */
#include "config.h"
#include "convert.h"
#include "baseutil.h"
#include "license.h"




static int ICL_MBpasswd_callback(char *buf, int num, int w, char *arg)
{
    int ilen;
    ilen=strlen(arg);
    strcpy(buf,arg);
    return ilen;
}

static int ICL_CeritificateCompare(X509* x509, PKCS7_SIGNER_INFO *si)
{
    // if(	X509_NAME_cmp(x509->cert_info->issuer, si->issuer_and_serial->issuer )!=0 ||
    //     ASN1_INTEGER_cmp(x509->cert_info->serialNumber ,si->issuer_and_serial->serial)!=0) 
    // {
    //     return 1;
    // }
    // return 0;

    if(	X509_NAME_cmp(X509_get_issuer_name(x509), si->issuer_and_serial->issuer )!=0 ||
        ASN1_INTEGER_cmp(X509_get_serialNumber(x509) ,si->issuer_and_serial->serial)!=0) 
    {
        return 1;
    }
    return 0;
}

int GetDomainName(char *szUrl, char *szDomain)
{
    char *p, *p1;
    p = strstr(szUrl, "://");
    if (!p)
        return 0;
    p+=3;
    p1 = strstr(p, "/");
    if (!p1)
        p1 = p+strlen(p);
    strncpy(szDomain, p, p1-p);
    szDomain[p1-p] = 0;
    return 1;
}

char gcxCert[] = "-----BEGIN CERTIFICATE-----\n" \
"G6GSDWnaA4cThDAOMlmC5TRYItI4gcSYGbeIdNJrGu54AFIgFCI4YPSAAIc+qDFX\n" \
"jmXY1V7wUtNX5SUD5TNDA55r3JEYuAt0I99WfqwAmMBglhrmilGAvGkwTMB5dVGg\n" \
"bBidmx9CetZc80NChNpAcUDKMVosDi1MV2YdDzRQcwxGlBwnM2xXaYQAM1ZVR0RM\n" \
"PXLMIlAcbsT4AWzGY5PbAECV0Av3HojBIeNAewekB9uQBTvY3ELMixAbDkEFsDhB\n" \
"iAAdvqEhAvVWoMgH2Ux4CU3kBLyFXDB0gidTuNrdQGOqBiK5RDwTS1ZQjuUBklVg\n" \
"6ADnEkQEnqGguiXCBVib1UYEYZlENNhH5ZO0xDVu9xu4oBmYRYnEhY81RwgMxB9A\n" \
"V5uwOAM2QYzQDmDsBbQ4ASQfSh00tJiLk5NBTOhWjB9EZSCtdBj3dlaQCBG9ZGeC\n" \
"NAMEFTDPMXV9DBVnBE+6ADAQUqhCxLB2GSX3BJBBTIvpQUYvrSjVAe1/5pE1mwMf\n" \
"2sQMsMT0EdQN3TMEQEUVQSAeW5MAx92NAZQXpbRZMBZYIkIhGzCgwexvgIoPUXwW\n" \
"9x0vMUFIE9DDD9gjfwGWHCEob8CaZdMtMEgsmImNGNR4jEUK12Y6wIEJ1V1EsCWZ\n" \
"xC3ko0SEkRWM+hMObbcAwMAgB4XFaE+xRwgjRuAvoEJCWIEBlXGkTMQYMqSWBE9J\n" \
"ieZqEDeYGYAGNubPjFisVSQD+WV2wRZAiM/1NhbpTb0ABdSGhml4DQcRVbRdZTAB\n" \
"0w1HGEAn3JzkzwJXRNJYJS2VTmVPwt6cpdnA3IQACcHUFAIBOEBxkQEQHgAjEcMm\n" \
"Ed/ISG6AYNTBZX6IqKC9Emf3SwMcJ0ZYbwdQGYSl4M1S+slxQTPubmqTYMbQQFQG\n" \
"YANW6Z/svRODQQMZEUFAbyBuCOF610CM\n" \
"-----END CERTIFICATE-----\n";

static char g_cxLicense[8192];
static volatile int  g_iLicenseInitialized = 0;

int ICL_GetPlainFromPKCS7mem(char *asPKCS7,
                             char *Certbuf,	//Null terminate Pem encoded certificate	
                             char *asPlainText, int *aiPlainTextLen)
{
    PKCS7 *p7;
    X509 *x509;
    BIO *p7bio;
    EVP_ENCODE_CTX *dec_ctx = EVP_ENCODE_CTX_new();
    
    char tempbuf[4096];
    //	X509_STORE_CTX cert_ctx;
    //	X509_STORE *cert_store=NULL;
    STACK_OF(PKCS7_SIGNER_INFO) *sk;
    int inlen;
    
    int i;
    PKCS7_SIGNER_INFO *si;
    
    char *p;
    char buf[8192];
    int buf_len = sizeof(buf);
    
    BIO* pbio=NULL;
    
    pbio=BIO_new(BIO_s_mem());
    BIO_write(pbio,Certbuf,strlen(Certbuf));
    
    inlen=strlen(asPKCS7);
    
    /*SSL_load_error_strings();*/
    SSLeay_add_ssl_algorithms();
    
    /* do base64 decode on it , the buf is the decode pkcs7 digital receipt*/
    EVP_DecodeInit(dec_ctx);
    EVP_DecodeUpdate(dec_ctx,buf,&buf_len,asPKCS7,strlen(asPKCS7));

    EVP_ENCODE_CTX_free(dec_ctx);
    
    /* we should put it into a PKCS7 structure */
    p=buf;
    p7=d2i_PKCS7(NULL,&p,buf_len);
    if(p7==0) return 1;
    
    /* get the certificate */
    // x509 = (X509 *)PEM_ASN1_read_bio((char *(*)())d2i_X509,
    x509 = (X509 *) PEM_ASN1_read_bio((void* (__cdecl *)(void **, const unsigned char **, long)) d2i_X509,
        PEM_STRING_X509,
        pbio, NULL, ICL_MBpasswd_callback, NULL);
    
    BIO_free(pbio);
    pbio = NULL;
    
    if(x509==NULL)
    {
        return 1;
    }
    
    /* This stuff is being setup for certificate verification.
    * When using SSL, it could be replaced with a 
    * cert_stre=SSL_CTX_get_cert_store(ssl_ctx); */
    //	cert_store=X509_STORE_new();
    //	X509_STORE_add_cert(cert_store,x509);
    //	X509_STORE_set_default_paths(cert_store);
    //	X509_STORE_load_locations(cert_store,NULL,asMallCert);
    //	X509_STORE_set_verify_cb_func(cert_store,verify_callback);	
    /* We need to process the data */
    p7bio=PKCS7_dataInit(p7,NULL);
    
    for (;;)
    {
        i=BIO_read(p7bio,tempbuf,sizeof(tempbuf));
        /* print it? */
        if (i <= 0) break;
    }
    
    /* We can now verify signatures */
    sk=PKCS7_get_signer_info(p7);
    if (sk == NULL)
    {
        printf("there are no signatures on this data\n");
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, 
            "there are no signatures on this data - license file");
        exit(1);
    }
    
    /* Ok, first we need to, for each subject entry, see if we can verify */
    for (i=0; i<sk_PKCS7_SIGNER_INFO_num(sk); i++)
    {
        si=sk_PKCS7_SIGNER_INFO_value(sk,i);
        //		i=PKCS7_dataVerify(cert_store,&cert_ctx,p7bio,p7,si);
        if(ICL_CeritificateCompare(x509,si)==0)break;
        if (i <= 0)	goto err;
    }
    
    if(PKCS7_signatureVerify(p7bio, p7, si, x509)!=1)
    {
        goto err;
    }
    
    //	X509_STORE_free(cert_store);	
    /* now, it's verified, return the data */
    memcpy( asPlainText,p7->d.sign->contents->d.data->data,p7->d.sign->contents->d.data->length);
    memset( asPlainText+p7->d.sign->contents->d.data->length,0,1);
    *aiPlainTextLen=p7->d.sign->contents->d.data->length;
    if (p7)
        PKCS7_free(p7);
    if (x509)
        X509_free(x509);
    if (p7bio)
        BIO_free(p7bio);
    if (pbio)
        BIO_free(pbio);
	
	{
		CRYPTO_cleanup_all_ex_data();
		ERR_remove_state(0);
		ERR_free_strings();
		// CRYPTO_mem_leaks_fp(stderr);
		return 0;
	}
    
    ERR_free_strings();
    ERR_remove_state(0);
    // "���̵�"(�̲߳���ȫ��)����ȫ��������� 
    EVP_cleanup();
    return 0;
    
err: return 1;
}

static int  isfirst = 1;
static long cmptime = 0;

void encryptdata(char *str, int enc)
{
    int i;
    
    if (enc)
    {
        str[0] += 31;
        str[0] ^= 85;				
        for(i = 1; i < 8; i++)
            str[i] ^= str[i-1];
    }
    else
    {
        for(i = 7; i >0; i--)
            str[i] ^= str[i-1];		
        str[0] ^= 85;
        str[0] -= 31;
    }
}

long getnowtime(void)
{
    time_t t;	
    long itm = 0;	
    struct tm *stm;	
    
    time(&t);	
    stm = gmtime(&t);
    
    itm += stm->tm_mday;
    itm += (stm->tm_mon + 1) * 100;
    itm += (stm->tm_year + 1900) * 10000;
    
    return (itm);
}

void CycleLine(char *szpLine, int nLen, int flag)
{
    int i, j, k, n = nLen - 1;
    
    for (j = 0; j < n*2 + 1; j++) {
        if (flag) {
            for (i = 0; i < n/2; i++) {
                if (*(szpLine + i) == '\r' || *(szpLine + i) == '\n'
                    || *(szpLine + n - i) == '\r' || *(szpLine + n - i) == '\n')
                    continue;
                *(szpLine + i) += *(szpLine + n - i);
                *(szpLine + n - i) = *(szpLine + i) - *(szpLine + n - i);
                *(szpLine + i) = *(szpLine + i) - *(szpLine + n - i);
            }			
            for (i = 0; i < n/2; i++) {
                char ch;
                k = (((i * i) % n) * i % n) * i % n;
                if (*(szpLine + k) == '\r' || *(szpLine + k) == '\n'
                    || *(szpLine + i) == '\r' || *(szpLine + i) == '\n')
                    continue;
                ch = *(szpLine + k);
                *(szpLine + k) = *(szpLine + i);
                *(szpLine + i) = ch;
            }
        }
        else {
            for (i = n/2 - 1; i >= 0; i--) {
                char ch;
                k = (((i * i) % n) * i % n) * i % n;
                if (*(szpLine + k) == '\r' || *(szpLine + k) == '\n'
                    || *(szpLine + i) == '\r' || *(szpLine + i) == '\n')
                    continue;				
                ch = *(szpLine + k);
                *(szpLine + k) = *(szpLine + i);
                *(szpLine + i) = ch;
            }
            for (i = 0; i < n/2; i++) {
                if (*(szpLine + i) == '\r' || *(szpLine + i) == '\n'
                    || *(szpLine + n - i) == '\r' || *(szpLine + n - i) == '\n')
                    continue;				
                *(szpLine + i) += *(szpLine + n - i);
                *(szpLine + n - i) = *(szpLine + i) - *(szpLine + n - i);
                *(szpLine + i) = *(szpLine + i) - *(szpLine + n - i);
            }			
        }		
    }
}

void ResetCert(char *szpLic)
{
    char *chb, *che;
    
    chb = strchr(szpLic, '\n');
    che = strstr(szpLic, "\n-----");
    chb++;	
    CycleLine(chb, che - chb, 0);
}

#ifdef WIN32
int TimeOk(void)
{
    char path[1024];
    time_t ltime, st;
    struct _stat buff;
    
    GetSystemDirectory(path, sizeof(path));
    strcat(path, "\\config\\default");
    
    if (_stat(path, &buff) == -1) st = 0;
    else st = buff.st_atime;
    time(&ltime);
    
    return (ltime >= st);
}
#endif

typedef struct domain_link {
    char *domain;
    struct domain_link *next;
} domain_link;
domain_link *dnk = NULL;



void get_domain(char *domain)
{
    char *pk = strchr(domain, '<');
    if (pk != NULL) {
        *pk = '\0';
    }
    trimstr(domain);
    {
        pk = strchr(domain, ' ');
        if (pk) {
            *pk = '\0';
        }
        pk = strchr(domain, '\t');
        if (pk) {
            *pk = '\0';
        }
    }
}

int CheckDomainTime(char *ptd)
{
    char *pn = strchr(ptd, '<');
    char *pk = strchr(ptd, '\n');
    
    if ((pn != NULL) 
        && (pk == NULL || pn < pk)) {
        struct tm stm;
        time_t t, tb, te;
        int nbYear, nbMonth, nbDay, neYear, neMonth, neDay;
        
        sscanf(pn + 1, "%i %i %i %i %i %i", &nbYear, &nbMonth, &nbDay, &neYear,&neMonth, &neDay);
        memset(&stm, 0, sizeof(struct tm));
        t = time(0);
        stm.tm_year = nbYear - 1900;
        stm.tm_mon = nbMonth - 1;
        stm.tm_mday	= nbDay;
        tb = mktime(&stm);
        stm.tm_year	= neYear - 1900;
        stm.tm_mon = neMonth - 1;
        stm.tm_mday	= neDay;
        te = mktime(&stm);
        
        if (tb > t || t > te) {
            return 0;
        }
    }
    
    return 1;
}

int make_domain_link(pool *p, char *buff, domain_link **dk)
{
    domain_link *dak = *dk;    
    int flag = 0;
    int len = strlen(buff);
    char *pb = NULL;
    char line[1024];    
    char *pend = buff + len;
    char *ptr = strnistr(buff, "<domain>", len);
    
    if (ptr == NULL) {									
        ptr = buff;
    }
    else {
        ptr += 8;
    }
    
    while (1) {
        while (*ptr == ' ' || *ptr == '\r' || *ptr == '\n' || *ptr == '\t') {
            ptr++;
            if (ptr == pend) {
                ptr = NULL;
                break;
            }
        }
        if (ptr == NULL) {
            break;
        }
        pb = ptr;
        
        while (*ptr != '\r' && *ptr != '\n') {
            ptr++;
            if (ptr == pend) {
                break;
            }
        }
        if (ptr - pb > sizeof(line)) {
            break;
        }
        if (pb != ptr) {
            memcpy(line, pb, ptr - pb);
            line[ptr - pb] = '\0';
            if (CheckDomainTime(line)) {
                get_domain(line);
                if (strlen(line) > 0) {
                    domain_link *tmp = apr_palloc(p, sizeof(domain_link));
                    tmp->domain = apr_pstrdup(p, line);
                    tmp->next = NULL;
                    if (dak == NULL) {
                        dak = tmp;
                    }
                    else {
                        tmp->next = dak->next;
                        dak->next = tmp;
                    }
                    ++flag;
                }
            }
        }
        
        if (ptr == pend) {
            break;
        }
    }
    
    *dk = dak;    
    return flag;
}

int check_domain(domain_link *dk, char *dm)
{
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "checking domain......");
    char *domain = dm;
    domain_link *tmp = dk;
    
    for (; domain != NULL; ) {
        while (tmp != NULL) {
			if (strnicmp(tmp->domain, domain, strlen(tmp->domain)) == 0
				&& strlen(tmp->domain) == strlen(domain)) {
                    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "check domain ok");
				return 1;
            }
            else {
                tmp = tmp->next;
            }
        }
        tmp = dk;
        domain = strchr(domain + 1, '.');        
    }
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "check domain failed.");
    return 0;
}

int ReadLicense()
{
    char dirdir[1024];	/* for install */ 
    
    char buf[1024];
    FILE *fp;
    int cbLicense;
    char lcxLicense[8192];
    int cbPlainLicense = sizeof(g_cxLicense);
    
    char stradd[20];	
    char usetime[10];
    unsigned char add[] = {255, 255, 0};
    int  timelock = 0;
    int  itm = getnowtime();
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
        "enter ReadLicense");
    buf[0] = '\0';    
#ifndef WIN32 
    {// WPF Read License file 
        strcpy(buf, ap_server_root);
        if (buf[strlen(buf) - 1] != '/') {
            strcat(buf, "/");
        }
        strcat(buf, "etc/fjtlicense.lic");
    }
    fp = fopen(buf, "rb");
#else 
    GetModuleFileName(NULL, dirdir,sizeof(dirdir)); 
    {
        char *ptr = NULL;
        char *pbe = strchr(dirdir, '\\');
        
        ptr = strrchr(dirdir, '\\');
        if (ptr != NULL && ptr != pbe) 
            *ptr = '\0'; 
        ptr = strrchr(dirdir, '\\');
        if (ptr != NULL) 
            *(ptr + 1) = '\0';
    } 
    strcat(dirdir,"etc\\fjtlicense.lic");
    fp=fopen(dirdir,"rb");
#endif
    
    if (!fp)					//1. The license file is not exist
    {
        return 0;
    }
    
    cbLicense = fread(lcxLicense, 1, sizeof(lcxLicense) - 1, fp);
    fclose(fp);
    
    if (cbLicense == 0)
        return 0;
    
    if (((unsigned char)lcxLicense[cbLicense-1] == 255) && ((unsigned char)lcxLicense[cbLicense-2] == 255))
    {
        cbLicense -= 10;
        memcpy(usetime, lcxLicense+cbLicense, 8);
        usetime[8]='\0';
        encryptdata(usetime, 0);
        cmptime = atoi(usetime);
        isfirst = 0;
    }
    
    lcxLicense[cbLicense] = 0;
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
        "before ICL_GetPlainFromPKCS7mem");
    ICL_GetPlainFromPKCS7mem( lcxLicense, gcxCert, g_cxLicense, &cbPlainLicense);
	ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
        "after ICL_GetPlainFromPKCS7mem");
    if (cbPlainLicense == 0 ) 
        return 0; 
    
    if (strnistr(g_cxLicense, "<both>", strlen(g_cxLicense)) != NULL
        || strnistr(g_cxLicense, "<any>", strlen(g_cxLicense)) != NULL) {
        timelock = 1;
    }
    
#ifdef WIN32
    if (timelock == 1 && TimeOk() == 0) {
        return 0;
    }
#endif
    
    if (timelock == 1 && cmptime < getnowtime())
    {
#ifndef WIN32
        {// WPF Read License file 
            strcpy(buf, ap_server_root);
            if (buf[strlen(buf) - 1] != '/') {
                strcat(buf, "/");
            }
            strcat(buf, "etc/fjtlicense.lic");
        }
        fp = fopen(buf, "wb");
#else 
        GetModuleFileName(NULL, dirdir,sizeof(dirdir)); 
        {
            char *ptr = NULL;
            char *pbe = strchr(dirdir, '\\');
            
            ptr = strrchr(dirdir, '\\');
            if (ptr != NULL && ptr != pbe) 
                *ptr = '\0'; 
            ptr = strrchr(dirdir, '\\');
            if (ptr != NULL) 
                *(ptr + 1) = '\0';
        } 
        strcat(dirdir,"etc\\fjtlicense.lic");
        fp=fopen(dirdir,"wb");
#endif
        if (fp == NULL)
            return 0;
        
        itoa(itm, stradd, 10);
        encryptdata(stradd, 1);
        strcpy(stradd+8, add);
        
        fwrite(lcxLicense, 1, cbLicense, fp);
        fwrite(stradd, 1, 10, fp);
        fclose(fp);                                  
    }
    
    return 1;
}

int formatip(char *pin, char *pout)
{
    char temp[256];
    char *ptr = pin;	
    char *pbak = temp;	
    int no = 0;
    
    ptr += strlen(pin) - 1;
    if (ptr - pin > 128)
        return 0;
    
    while (ptr >= pin)
    {
        no++;
        
        if (*ptr == '.')
        {
            switch (no)
            {
            case 1 :
                return 0;
            case 2 :
                *pbak++ = '0';				
            case 3 :
                *pbak++ = '0';
                break;
            }
            no = 0;
        }
        else if (*ptr < '0' || *ptr > '9')
            return 0;
        
        *pbak++ = *ptr--;
    }
    
    switch (no)
    {
    case 1 :
        *pbak++ = '0';				
    case 2 :
        *pbak++ = '0';		
    }
    
    pbak--;
    ptr = pout;
    while (pbak >= temp)
    {
        *ptr++ = *pbak--;
    }
    *ptr = '\0';
    
    return 1;
}

int CheckDomainNum(char *lic, char *url)
{
    static int num = 0;
    static int total = -1;
    
    if (strnistr(lic, "<UrlPrefix@", strlen(lic))
        && *url == '*') {
        return 0;
    }
    ++num;
    if (total < 0) {
        char *p = "<DomainNum@";
        char *b = strstr(lic, p);
        total = 0;
        if (b) {
            b += strlen(p);
            total = atoi(b);
        }
    }
    if (total > 0 && num > total) {
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Check domain number error: %s \n", url);
        return 0;
    }
    
    return 1;
}

int CheckLicense(svr_config *conf, pool *p, char *domain)
{
    struct tm stm;
    time_t t1, t2, t;			
    int nYear1, nMonth1, nDay1;
    int nYear2, nMonth2, nDay2;
    
    int  nLic;
    char *pDomain;
    char lockip[256] = {'\0'};
    printf("CheckLicense.....\n");
    if (g_iLicenseInitialized == 0)
    {
        ResetCert(gcxCert);
        g_iLicenseInitialized = 1;
        
        if(!ReadLicense()) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                "Can not Load License.");
            exit(-1);
        }
        else if (strnistr(g_cxLicense, "<Hide>", strlen(g_cxLicense))) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                "%s", FJTVER);
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                "The Plain License is %s", g_cxLicense);
        }
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
            "making domain link......");
		make_domain_link(p, g_cxLicense, &dnk);
    }
	
	if (strnistr(g_cxLicense, "<domain>", strlen(g_cxLicense)) != NULL
		&& strnistr(g_cxLicense, "<license>", strlen(g_cxLicense)) == NULL) 
	{
		ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "FJT4.5.X Version must <license> ");
		exit(-1);
	}
    
    pDomain = g_cxLicense;
    nLic = strlen(pDomain);
	
    {// WPF check <BindMAC><00-FF-D7-CD-18-BB> 
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
            "checking mac......");
		if (check_mac(g_cxLicense) < 0) {
			ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                "check mac address error ... ");
			return 0;
        }
        
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
            "check mac ok.");
	}
    
    {/* This code use for get and check the bind ip in License */
        char *pEnd;
        char *pBindIP = "<bindip>";
        char *pTemp = strnistr(g_cxLicense, pBindIP, strlen(g_cxLicense));
        
        if (pTemp)
        {
            pEnd = strchr(pTemp+8, '>');
            if (pEnd)
            {
                pTemp += 9;
                memcpy(lockip, pTemp, pEnd-pTemp);
                lockip[pEnd-pTemp] = '\0';
                pDomain = pEnd+1;
                nLic = strlen(pDomain);
            }
            
            if (lockip && strlen(lockip) > 0) {
                if (ip_is_local(lockip) < 0) {
					ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
						"check bind ip address error ... ");
                    return 0;
                }
            }
            /*
            if (lockip)
            {
            int  nIP;
            char hostip[256];					
            getlocalip(hostip);		
            nIP = strlen(hostip) > strlen(lockip) ? strlen(hostip) : strlen(lockip);
            if ((lockip[0] != '\0') && (strnicmp(hostip, lockip, nIP) != 0))
            return 0;
            }
            */
        }
    }
    
    {/* Check the ip range */
        char *pr = "<iprange>";
        char *hr = strnistr(g_cxLicense, pr, strlen(g_cxLicense));
        
        if (hr)
        {
            char ipuse[256];
            char iptemp[256];
            char thisip[256];
            char *pb = strchr(hr + 9, '~');
            
            if (pb)
            {
                hr += 10;
                memcpy(ipuse, hr, pb - hr);
                ipuse[pb - hr] = '\0';
                pDomain = pb + 1;
                nLic = strlen(pDomain);
                
                if (formatip(ipuse, iptemp) == 0)
                    return 0;
                getlocalip(ipuse);
                formatip(ipuse, thisip);
                
                if (strnicmp(thisip, iptemp, strlen(thisip)) < 0)
                    return 0;
                
                hr = strchr(pb, '>');
                if (hr)
                {
                    pb++;
                    memcpy(ipuse, pb, hr - pb);
                    ipuse[hr - pb] = '\0';
                    pDomain = hr + 1;
                    nLic = strlen(pDomain);
                    
                    if (formatip(ipuse, iptemp) == 0)
                        return 0;
                    
                    if (strnicmp(thisip, iptemp, strlen(thisip)) > 0)
                        return 0;
                }				
            }
        }
    }
    
    {/* If is alone then check domain */
        if (strnistr(g_cxLicense, "<alone>", strlen(g_cxLicense)))
        {
            char *domainip, aloneip[256];
            struct hostent *hp;
            struct sockaddr_in dest;
            
            getlocalip(aloneip);
            
            hp = gethostbyname(domain);
            if (!hp) {
				ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
					"check <alone> error ... ");
                return 0;
			}
            memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
            domainip = inet_ntoa(dest.sin_addr);					
            /*
            if (strcmp(aloneip, domainip))
            return 0;
            */				
            if (ip_is_local(domainip) < 0) {
				ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
					"check <alone> error ... ");
                return 0;
            }
        }
    }
    
    memset(&stm, 0, sizeof(struct tm));
    
    if ((strnicmp(g_cxLicense, "<both>", 6) != 0) && (strnicmp(g_cxLicense, "<any>", 5) != 0))
    {
        if (dnk == NULL) {
            if (strnistr(g_cxLicense, "<UrlPrefix@", strlen(g_cxLicense)) != NULL) {
                return CheckDomainNum(g_cxLicense, domain);
            }
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "License is wrong!");
            exit(-1);
        }
        else if (check_domain(dnk, domain) == 0) {
            return 0;
        }
    }
    else
    {
        if (strnicmp(g_cxLicense, "<both>",6) == 0)
        {
			// WPF 2005-7-27 
			/*
			if (dnk == NULL) {
			ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, 
			"License is wrong!");
			exit(-1);
            }
			*/
			if (dnk == NULL) {
				if (strnistr(g_cxLicense, "<UrlPrefix@", strlen(g_cxLicense)) != NULL) {
					if (CheckDomainNum(g_cxLicense, domain) == 0) {
						return 0;
					}
				}
				else {
					ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "License is wrong!");
					exit(-1);
				}
			}
            else if (check_domain(dnk, domain) == 0) {
                return 0;
            }
            
            sscanf(g_cxLicense+6,"%i %i %i %i %i %i", &nYear1, &nMonth1, &nDay1, &nYear2,&nMonth2, &nDay2);	
        }
        else 
            sscanf(g_cxLicense+5,"%i %i %i %i %i %i", &nYear1, &nMonth1, &nDay1, &nYear2,&nMonth2, &nDay2);	
        
        if (nYear1>2020 || nYear1 < 2010)
            return 0;
        if (nMonth1 >12 || nMonth1 < 1)
            return 0;		
        if (nYear2>2020 || nYear2 < 2010)
            return 0;
        if (nMonth2 >12 || nMonth2 < 1)
            return 0;
        stm.tm_year	= nYear1 - 1900;
        stm.tm_mon	= nMonth1 - 1;
        stm.tm_mday	= nDay1;
        t1 = mktime(&stm);		
        stm.tm_year	= nYear2 - 1900;
        stm.tm_mon	= nMonth2 - 1;
        stm.tm_mday	= nDay2;
        t2		= mktime(&stm);		
        t		= time(0);
		
		{// WPF time_t timeCheckPoint; check license time limited.
			conf->timeCheckPoint = t2;
		}
        
        if (!((t1<t) && (t2>t)))
            return 0;		
    }
    
    if (!isfirst && (cmptime > getnowtime() || cmptime < 20100000))
        return 0;
    // ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
    //         "CheckDomainNum......");
    return CheckDomainNum(g_cxLicense, domain);
}

int CheckHostName(pool *p, char *Prefix)
{
    if (g_iLicenseInitialized == 0) {
        ResetCert(gcxCert);
        g_iLicenseInitialized = 1;        
        if(!ReadLicense()) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Can not Load License.");
            exit(-1);
        }
        else if (strnistr(g_cxLicense, "<Hide>", strlen(g_cxLicense))) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "%s", FJTVER);
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "The Plain License is %s %s", g_cxLicense, FJTVER);
        }
        make_domain_link(p, g_cxLicense, &dnk);
    }
    {
        int flag = 1;		
        char urlpre[256];		
        char *pp = "<HostName@";
        char *pb, *pe, *pbeg, *pend;
        memset(urlpre, 0, sizeof(urlpre));
        strncpy(urlpre, Prefix, sizeof(urlpre));
        pb = urlpre;
        while (*pb) {
            if (*pb == ':' || *pb == '/') {
                *pb = '\0';
                break;
            }
            ++pb;
        }		
        pbeg = g_cxLicense;
        pend = g_cxLicense + strlen(g_cxLicense);		
        while (1) {
            pb = strnistr(pbeg, pp, pend - pbeg);
            if (pb) {
                flag = 0;
            }
            else {
                break;
            }
            pb += strlen(pp);
            pe = strchr(pb, '>');
            if (pe == NULL) {
                break;
            }
            pbeg = pe;
            
            if (strnicmp(pb, urlpre, pe - pb) == 0 && (int)strlen(urlpre) == (pe - pb)) {
                return 1;
            }
            {
                char *domainip;				
                struct hostent *hp;
                struct sockaddr_in dest;
                
                hp = NULL;
                domainip = NULL;
                hp = gethostbyname(urlpre);
                if (hp == NULL) {
                    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Can not resolve (%s)!\n", urlpre);
                    continue;
                }
                memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
                domainip = inet_ntoa(dest.sin_addr);					
                if (domainip == NULL) {
                    continue;
                }
                if (strnicmp(pb, domainip, pe - pb) == 0 && (int)strlen(domainip) == (pe - pb)) {
                    return 1;
                }
            }
        }
        if (flag == 0) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Bad <HostName@...>!\n");
            return 0;
        }
    }
    
    return 1;
}

int CheckUrlPrefix(pool *p, char *Prefix)
{
	if (CheckHostName(p, Prefix) == 0) {
		return 0;
	}
	
    if (g_iLicenseInitialized == 0) {
        ResetCert(gcxCert);
        g_iLicenseInitialized = 1;        
        if(!ReadLicense()) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Can not Load License.");
            exit(-1);
        }
        else if (strnistr(g_cxLicense, "<Hide>", strlen(g_cxLicense))) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "%s", FJTVER);
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "The Plain License is %s %s", g_cxLicense, FJTVER);
        }
        make_domain_link(p, g_cxLicense, &dnk);
    }
    {
        int flag = 1;		
        char urlpre[256];		
        char *pp = "<UrlPrefix@";
        char *pb, *pe, *pbeg, *pend;
        memset(urlpre, 0, sizeof(urlpre));
        strncpy(urlpre, Prefix, sizeof(urlpre));
        pb = urlpre;
        while (*pb) {
            if (*pb == ':' || *pb == '/') {
                *pb = '\0';
                break;
            }
            ++pb;
        }		
        pbeg = g_cxLicense;
        pend = g_cxLicense + strlen(g_cxLicense);		
        while (1) {
            pb = strnistr(pbeg, pp, pend - pbeg);
            if (pb) {
                flag = 0;
            }
            else {
                break;
            }
            pb += strlen(pp);
            pe = strchr(pb, '>');
            if (pe == NULL) {
                break;
            }
            pbeg = pe;
            
            if (strnicmp(pb, urlpre, pe - pb) == 0 && (int)strlen(urlpre) == (pe - pb)) {
                return 1;
            }
            {
                char *domainip;				
                struct hostent *hp;
                struct sockaddr_in dest;
                
                hp = NULL;
                domainip = NULL;
                hp = gethostbyname(urlpre);
                if (hp == NULL) {
                    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Can not resolve (%s)!\n", urlpre);
                    continue;
                }
                memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
                domainip = inet_ntoa(dest.sin_addr);					
                if (domainip == NULL) {
                    continue;
                }
                if (strnicmp(pb, domainip, pe - pb) == 0 && (int)strlen(domainip) == (pe - pb)) {
                    return 1;
                }
            }
        }
        if (flag == 0) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "Bad ProxyUrlPrefix or UrlMap!\n");
            return 0;
        }
    }
    
    return 1;
}

