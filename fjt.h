#ifndef FJT_VERSION_H
#define FJT_VERSION_H

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
/* // #define ENCRYPT_WORDS */


#define FJT_BASEREVISION  "5.0.29"


#ifdef  ENCRYPT_WORDS
#define ENWS " (ENCRYPT)"
#else  
#define ENWS
#endif
#define FJT_BASEPRODUCT  "FJT_XP"
#define FJTECHO  "("  FJT_BASEPRODUCT  FJT_BASEREVISION  ")"  ENWS
#define FJTVERSION  FJT_BASEPRODUCT  " Version "  FJT_BASEREVISION
#define FJTVER  FJTVERSION  ENWS


#endif


/* 
* WPF Apache2.0 
* ---------------- 
*     FILE
* my_proxy
* mod_proxy.h 
* mod_proxy.c 
* proxy_http.c 
* ---------------- 
* mod_cache.h ?
* mod_cache.c | modules\experimental 
* mod_disk_cache.h ?
* mod_disk_cache.c | modules\experimental 
* -----------------
*     PROJECT
* mod_ssl
* mod_proxy 
* mod_proxy_http 
* ------------------
*     HEAD 
* /I "e:\Openssl2.0\include" /I "e:\Openssl2.0\include\openssl"
*     LIB
* /libpath:"E:\Openssl2.0\lib" 
* ------------------
* ,e:\Openssl2.0\include,e:\Openssl2.0\include\openssl
* E:\Openssl2.0\lib
* libeay32.lib ssleay32.lib ADVAPI32.LIB
* c:\Program Files\Microsoft Visual Studio\VC98\Lib\ADVAPI32.LIB
* -----------------
* ap_release.h | include - Apend FJT Ver  
* nt_eventlog.c | server\mpm\winnt - Report Infomation
* -----------------
*     RES
* build\win32 - 
* Apache	 .rc	logo.ico 
* support\win32 - 
* ApacheMoniter	.rc	 HeaderTitle.bmp Modify .c 
*/ 



/* /////////////////////////////////////////////////////////////////
/// httpd-2.2.14 */

/***********************
* apr_dbd_odbc.c   
* typedef  INT32  SQLLEN;
* typedef  UINT32 SQLULEN;
***********************/

/***********************
Open httpd-2.2.11\srclib\apr-util\include\apr_ldap.h file and delete following code��
#error Support for LDAP v2.0 toolkits has been removed from apr-util. Please use an LDAP v3.0 toolkit.

  apr_ldap.h
  / ** WPF
  #if LDAP_VERSION_MAX <= 2
  #error Support for LDAP v2.0 toolkits has been removed from apr-util. Please use an LDAP v3.0 toolkit.
  #endif 
  ** /
  
	
	  "WPF HERE" // LDAP_VENDOR_NAME
	  
***********************/

/* // apr_dbd_odbc.c 
// httpd-2.2.11\srclib\apr-util\include\apr_ldap.h */


/***********************
install Platform SDK xpsp2
***********************/




