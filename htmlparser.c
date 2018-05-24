#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "mod_proxy.h"
#include "config.h"
#include "memstream.h"
#include "stack.h"
#include "htmlparser.h"
#include "baseutil.h"
#include "convert.h"

#ifndef  WIN32 
#include "errno.h"
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define  system MY_SYSTEM 

int MY_SYSTEM(char *command) 
{
	int pid, status;
	extern char **environ;
	
	if (command == 0)
		return 1;
	
	pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0) 
	{
		char *argv[4];
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;
		execve("/bin/sh", argv, environ);
		exit(127);
	}
	do 
	{
		if (waitpid(pid, &status, 0) == -1) 
		{
			if (errno != EINTR)
				return -1;
		} else
			return status;
	} while(1);
}
#endif

extern void getdir(char *dir,int len);

int ProcessUnicode(config *pcon, ConvertCtx *pctx, char *pins, int nsize)
{
	int no = nsize + 1;
	char *ptemp = NULL;
	
	if (no > pctx->nOutput)
	{
		pctx->pcOutput = apr_palloc(pctx->p, LOGALLOCSIZE(no));
		pctx->nOutput = no;
	}
	ptemp = pctx->pcOutput;
	no = nsize;
	
	if (pctx->cUnicodeLeft[0] != '\0')
	{
		*ptemp = pctx->cUnicodeLeft[0];
		memcpy(ptemp + 1, pins, nsize);
		pctx->cUnicodeLeft[0] = '\0';		
		no++;
	}
	else
		memcpy(ptemp, pins, nsize);
	
	if (no % 2)
	{
		pctx->cUnicodeLeft[0] = *(ptemp + no - 1);
		no--;
	}
	no = ConvertFromUnicodeEx(pcon->m_iFromEncode, pctx->nUnicodeFormat, ptemp, no, pins);
	
	return no;	
} 

int ChangeScript(pool *apool, ConvertCtx *pctx, memstream *apwrite, config *pconf, char *pbuf, int nsize)
{
	char *pend;
	char *pcur;
	char *pathb;
	char *pathe;
	char *pNewUrl;
	int nNewUrl;
	int re;
	int oldconf;
	
	/* 2002-01-10 */
	char *pboth = NULL;
	
	// WPF	2002-4-6
	int iPlug = -1;
	// WPF 2003-3-13 
	int iNotAdd = 0;
	
	if ((pbuf==NULL) || (nsize==0)) {
		return 0;
	}
	
	/*
	WPF 2003-4-5
	CheckLeftBuffer(pctx, pconf, &pbuf, &nsize);
	*/
	
	pcur = pbuf;
	pend = pbuf + nsize - 1;
	pathb = pcur;
	
	if (pconf->m_iProcessScriptLevel < 2) 
	{// WPF 2002-6-20
		ReplaceHTTPAndConvert(pctx, apwrite, pconf, pcur, nsize);
		return 0;
	}
	
	while(pathb < pend)
	{
		int preflag = 0;
		
		if (*pathb=='.' && *(pathb+1)=='/') {
			preflag = 1;
		}
		else if (*pathb=='.' && *(pathb+1)=='.' && *(pathb+2)=='/') {
			preflag = 2;
		} 
		else if (*pathb == '/' && *(pathb+1)!='\t' && *(pathb+1)!=' ' && *(pathb+1)!='\r' && *(pathb+1)!='\n' && *(pathb+1)!='/' && *(pathb+1)!='*') {
			preflag = 3;
		}
		else if ((strnicmp(pathb,"http://",7)==0) || (strnicmp(pathb,"https://",8)==0)) {
			preflag = 4;
		}
		else if (*pathb == '\\' && *(pathb + 1) == '/') {
			preflag = 5;
		}
		
		if (preflag != 0) 
		{
			char *deli = NULL;
			int iTwoDot = 0;
			
			{//	WPF 2002-7-26 
				if (*(pathb - 1) == '\\') {
					pathb += 2;
					continue;
				} 
				else if (preflag == 1) 
				{
					char *pdot = pathb + 2; 
					while ((pdot < pend) && (*pdot == '.' || *pdot == '/')) {
						pdot++;
					}
					if (*(pdot - 1) != '/') 
					{
						pathb = pdot;
						continue;
					}
					
					{// WPF 2003-5-30 indexOf("./") and replace("./") 
						char *pir = pathb - 48;
						if (pir > pbuf 
							&& (strnistr(pir, "indexOf(", 48)
							|| strnistr(pir, "replace(", 48))) {
							pir += 8;
							while (pir < pathb) {
								if (*pir == ')') {
									break;
								}
								pir++;
							}
							if (pir == pathb) {
								pathb += 2;
								continue;
							}
						}
					}
				}
				// WPF 2002-8-22
				else if ((*pathb == '/') && (*(pathb + 1) == 'g') && ((*(pathb + 2) == ';') || (*(pathb + 2) == ',')))
				{
					pathb += 3;
					continue;
				}
				// WPF 2002-8-26
				else if ((*pathb == '/') && (*(pathb + 1) == 'g') && (*(pathb + 2) == 'i')
					&& ((*(pathb + 3) == ';') || (*(pathb + 3) == ','))) {
					pathb += 3;
					continue;
				}
				// WPF 2002-9-17
				else if ((*pathb == '/') && (*(pathb + 1) == '^')) {
					pathb += 3;
					continue;
				} 
				// WPF 2003-5-12 
				else if ((*pathb == '/') && ((*(pathb + 1) == '\\')
					|| (*(pathb - 1) == '\\'))) {
					pathb += 2; 
					continue; 
				} 
				else if (*(pathb + 1) == ':') {
					pathb += 2;
					continue;
				}
				else if (preflag == 2) {
					iTwoDot = 1;
				}
				else if (*(pathb + 4) == ':' || *(pathb + 5) == ':') {
					iTwoDot = 1;
				}
			}
			
			/*
			{
			must discuss
			indexOf("./") and replace("./")
			}
			*/
			
			deli = pathb - 1;
			pathe = pathb;
			if (deli < pbuf)
			{
				deli = NULL;
			}
			
			{// WPF 2003-3-13 
				if (iPlug == 1 && (pend - pathb > 8)
					&& (strnicmp(pathb, "http://", 7) == 0 || strnicmp(pathb, "https://", 8) == 0)) {
					iPlug = -1;
					iNotAdd = 1;
				}
				else {
					iNotAdd = 0;
				}
			} 
			
			// WPF	2002-4-6
			if ((deli && (deli == pboth)) || (iPlug == 1))
			{
				pathb++;
				continue;
			}
			
			if ((deli!=NULL) && ((*deli == '\'') || (*deli == '\"')))
			{
				{// WPF 2003-7-31
					char *psh = deli - 1;
					if (deli > pbuf && *psh == '/') {
						if (psh > pbuf && *(psh - 1) == '*') {
							// NULL;
						}
						else {
							pathb++;
							continue;
						}
					}  
					else if (deli > pbuf && *psh == '(' && preflag == 3) {
						// <a href="javascript:turnOn('/cpyfw/jryw/xtyw?flag=01?lctn=0')">
						// WPF 2007-3-23 
						//
						// WPF 10:48 2008-4-10
						// pathb++;
						// continue;
					}
				}
				
				{// WPF	2002-4-6
					if ((deli > pbuf) && (*(deli - 1) == '\\')
						&& strnistr(deli, "/g ", pend - deli)) // WPF 2002-4-10
					{
						pathb++;
						continue;
					}
				}
				
				pathe = pathb;
				while((pathe - pathb < 512) && (pathe <= pend))
				{
					if (*pathe == *deli ||
						*pathe == '"') 
						// <img src='/wps/themes/html/images/index_nav"+mainLayer[i]+".gif'
						// WPF 2007-3-23 
					{
						pboth = pathe; 
						
						// WPF 2003-6-2
						if (iTwoDot == 0 && pathe < pend) {
							char ph = *(pathe + 1);
							if (ph != ';' && ph != ' ' && ph != ')' 
								&& ph != '&' && ph != '+' && ph != ',' 
								&& ph != '\'' && ph != '"'
								&& ph != '>' && ph != '\r' && ph != '\n' && ph != '\t'
								&& ph != '<' // 2007.10.26
								) {
								pathe = NULL;
								break;
							}
						}
						pathe--;
						if (*pathe == '\\')
						{
							pathe--;
						}
						break;
					} 
					// WPF 2004-08-11
					else if (*pathe == '|') {
						break;
					}
					// WPF 2007-3-26 || 2007-10-11
					else if (*pathe == '*' || *pathe == '>') {
						pathe = NULL;
						break;
					}
					
					pathe++;
				}
				
				// WPF 2003-3-5
				if (pathe == NULL) {
					pathb++;
					continue;
				}
				
				{// WPF 2003-1-13 
					if (pathe > pend) {
						pathe = pend;
					}
				} 
				
				if (pathe - pathb > 512)
				{
					pathb++;
					continue;
				}
			}
			else if ((deli!=NULL) && (*pathb=='/') && ( *deli!='"' && *deli!='\'' && *deli!='='))
			{
				char *ptemp;
				
				for(ptemp = deli; ptemp >= pbuf; ptemp--)
					if ((*ptemp != ' ') || (*ptemp !='\t'))
						break;
					if ((ptemp > pathb) && (*pathb=='/') && ( *ptemp!='"' && *ptemp!='\'' && *ptemp!='='))
					{
						pathb++;
						continue;
					}
			}
			else    /* there is no delimitor available */
			{// WPF 2004-6-3
				int nok = 1;
				pathe = pathb;
				while ((pathe - pathb < 512) && (pathe < pend)) {
					if (*pathe == ';' || *pathe == ' ' || *pathe == ')' 
						|| *pathe == '&'  || *pathe == ',' || *pathe == '\t'
						|| *pathe == '\'' || *pathe == '"'  || *pathe == '>' 
						// WPF 2005-6-29 
						// || *pathe == '<'  || *pathe == '\r' || *pathe == '\n') {                
						|| *pathe == '\r' || *pathe == '\n') {                
						--pathe;
						break;
					}
					// WPF 2004-6-9
					else if (*pathe == '*' || *pathe == '[' || *pathe == ']' || *pathe == '\\'
						// WPF 2005-6-29 
						// || *pathe == '{' || *pathe == '}' || *pathe == '+') { 
						|| *pathe == '<' || *pathe == '{' || *pathe == '}' || *pathe == '+') { 
						nok = 0;
						break; 
					} 
					++pathe;
				}
				if (nok == 0) {
					pathb++;
					continue;
				}
			}
			
			/* now we got the link, call changeurl*/
			if (*pathe=='\\')
			{
				pathe--;
			}
			if (*pathb=='/' && pathe==pathb)
			{
				pathb++;
				continue;
			} 
			
			{// WPF 2003-4-3
				if (*pathb == '/' && *(pathb - 1) == '<') {
					++pathb;
					continue;
				}
			} 
			
			oldconf = pconf->m_iChangeChineseLevel;
			pconf->m_iChangeChineseLevel = pconf->m_iScriptChangeChineseLevel;
			
			/*
			{// WPF log info DEBUG !
			FILE *fp = NULL;
			int n = pathe - pathb + 1 + 20;
			fp = fopen("d:\\log.txt", "a+");
			if (fp != NULL) {
			fwrite(pathb - 10, 1, n, fp);
			fwrite("------\r\n", 1, 8, fp);
			fclose(fp);
			}
			}
			//*/
			
			{// WPF 2003-3-13 
				if (iNotAdd == 1) {
					int bkts = pconf->m_iAddTrailingSlash;
					pconf->m_iAddTrailingSlash = 0;
					re = ChangeUrl(apool, pctx, pconf, pathb, pathe - pathb + 1, &pNewUrl, &nNewUrl);
					pconf->m_iAddTrailingSlash = bkts;
				}
				else {
					re = ChangeUrl(apool, pctx, pconf, pathb, pathe - pathb + 1, &pNewUrl, &nNewUrl);
				}
			}
			
			pconf->m_iChangeChineseLevel = oldconf;
			
			if (!re)
			{	
				pathb++;
				continue;
			}
			else
			{
				convert(pconf->m_usetable, pcur,pathb-pcur, apwrite, pconf,pctx);
				memstream_write(apwrite, pNewUrl, nNewUrl);
				pcur = pathe+1;
				pathb = pathe + 1;
			}
		}
		else
		{
			{// WPF	2004-2-19
				if (iPlug == -1)
				{
					if (*pathb == '+')
						iPlug = 0;
				}
				else if (*pathb == '\'' || *pathb == '\"')
				{
					if (iPlug == 0)
						iPlug = 1;
					else
						iPlug = -1;
				}
				if (*pathb == ';' || *pathb == '}' || *pathb == '<') {
					iPlug = -1;
				}
			}
			
			if (*pathb < 0)
				pathb+=2;
			else if (*pathb == '/' && *(pathb+1)=='/')
			{
				while(pathb<pend)
				{
					if (*pathb == '\r' || *pathb=='\n') break;
					pathb++;
				}
			}
			else if (*pathb == '/' && *(pathb+1)=='*')
			{
				while(pathb<pend)
				{
					if (*pathb == '*' && *(pathb+1)=='/') break;
					pathb++;
				}
			}
			else
				pathb++;
		}
	}
	
	// convert(pconf->m_usetable, pcur,pathb-pcur+1, apwrite, pconf,pctx);
	{// WPF 2003-1-13 
		int left = pend - pcur + 1;		
		
		if (left > 0) {
			convert(pconf->m_usetable, pcur, left, apwrite, pconf, pctx);
		}
	}
	return 0;
}

int UrlMapHTTP(pool *apool, ConvertCtx *apCtx, config *pconf, char *pabsurl,int absurl_len, char **purl, int *len)
{
	array_header *reqhdrs_arr; 
	table_entry *reqhdrs; 
	int i; 
	
	reqhdrs_arr = (array_header*) apr_table_elts(pconf->m_pUrlMap); 
	if (!reqhdrs_arr) {
		return 0;
	}
	reqhdrs = (table_entry *)reqhdrs_arr->elts;
	
	for (i = 0; i < reqhdrs_arr->nelts; i++) {
		int length = 0;
		char *fromUrl, *toUrl; 
		fromUrl = reqhdrs[i].key; 
		toUrl = reqhdrs[i].val; 
		length = strlen(fromUrl);
		if (length < absurl_len && strnicmp(fromUrl, pabsurl, length) == 0) {
			*purl = toUrl;
			*len = strlen(toUrl);
			return length;
		}
	}
	
	return 0;     
}


int write_to_memstream_escape_slash(memstream *ms, char *str)
{
	int r = 0;
	
	while (*str != '\0') {
		if (*str == '/') {
			memstream_write(ms, "\\/", 2);
			r += 2;
		}
		else {
			memstream_write(ms, str, 1);
			++r;
		}
		++str;
	}
	
	return r;
}


int ReplaceHTTPAndConvert(ConvertCtx *pctx, memstream *apwrite, config *pconf, char *pbuf, int nsize)
{
	char *p;
	char *pend;
	char *pcur;    
	int  i = 0;
	int  slash = 0;
	char *str = NULL;
	
	pcur = pbuf;
	pend = pbuf + nsize - 1;    
	if (pctx->nNotChangeTextboxUrl > 0 || pconf->m_iNotReplaceUrl == 1 || pctx->fjtignoreurl == 1) {
		convert(pconf->m_usetable, pcur, pend - pcur + 1, apwrite, pconf, pctx);
		--pctx->nNotChangeTextboxUrl;
		return 1;
	}
	int len = pend - pcur + 1;
	p = strnistr(pcur,"url(",len);
	if(p > pcur){
	    char c = *(p-1);
	    //如果url前面不是分隔符，比如setUrl(，这种情况就不应该转
	    if(c!=' ' && c!=';' && c!=',' && c!='\t' && c!='\n' && c!='\r' && c!='\'' && c!='"' && c!='('){
	        p = NULL;
	    }
	}
	char *purlEnd = NULL;
	if(p!=NULL){
		purlEnd = strnistr(p,")",pend - p + 1);
		if(purlEnd == NULL){
			p = NULL;
		}
		else{
			//如果在 ")"之前有引号，也不转，很可能是javascript的表达式
			char *ptemp = p;
			while(ptemp < purlEnd){
				if(*ptemp == '"' || *ptemp== '\''){
					p = NULL;
					break;
				}
				ptemp++;
			}
		}
	}


	if(p!=NULL && purlEnd !=NULL){

		if(*(p-1)=='\'' || *(p-1)=='\"'){
		    //寻找到匹配的
            char c = *(p-1);
            char *pquoteEnd = p;
            while(*pquoteEnd != c && pend > pquoteEnd){
                pquoteEnd+=1;
            }

            if(purlEnd > pquoteEnd){
            	//这不是一个完整的url(结构
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,p - pcur);
				memstream_write(apwrite,p, 4);
				pcur = p + 4;
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,pend - pcur + 1);
				return 1;
            }
		}

		if(purlEnd!=NULL){
		    //首先找到开始的引号
            p = p+4; //should point to pos after url(
            while(*p == ' ' || *p =='\t' || *p=='\r' || *p=='\n' || *p=='\'' || *p=='\"' || *p==';'){
                p=p+1;
                if(p >= purlEnd){
                    break;
                }
            }
            purlEnd = purlEnd -1;
            while(*purlEnd == ' ' || *purlEnd =='\t' || *purlEnd=='\r' || *purlEnd=='\n' || *purlEnd=='\'' || *purlEnd=='\"' || *purlEnd==';'){
            	purlEnd--;
				if(p >= purlEnd){
					break;
				}
            }
			if(p<purlEnd){
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,p - pcur);
				char *pNewUrl;
				int nNewUrl;
				ChangeUrl(pctx->p, pctx, pconf, p,purlEnd - p + 1, &pNewUrl, &nNewUrl);
				memstream_write(apwrite,pNewUrl,nNewUrl);
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,purlEnd + 1,pend - purlEnd);
				return 1;
			}
			else{
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,p - pcur);
				pcur = p;
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,pend - pcur + 1);
				return 1;
            }
		}
		else{
            ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,p - pcur);
			memstream_write(apwrite,p, 4);
			pcur = p + 4;
			ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,pend - pcur + 1);
			return 1;
		}

	}

	p = strnistr(pcur, "http://", pend - pcur + 1);
	char *pdomain = NULL;
	if(p==NULL){
		p = strnistr(pcur, "https://", pend - pcur + 1);
		if(p!=NULL){
			pdomain = p + strlen("https://");
			str=pconf->m_pcSUrlPrefix;
		}
	}
	else{
		pdomain = p + strlen("http://");
		str=pconf->m_pcUrlPrefix;
	}

	if(p!=NULL){
		char *purlEnd = NULL;
		if(p>pcur) {
			char c = *(p - 1);

			if (c == '\'') {
				//寻找'
				purlEnd = strnistr(p, "'", pend - p + 1);
			} else if (c == '\"') {
				purlEnd = strnistr(p, "\"", pend - p + 1);
			} else if (c == '(') {
				purlEnd = strnistr(p, ")", pend - p + 1);
			}
		}
		if(purlEnd!=NULL){
		    if((*purlEnd=='\'' || *purlEnd=='\"') && (*(purlEnd-1)=='\\')){
		        //这是针对javascript里面 "href=\"http://www.csv.com\" ..., 类似的情况
		        purlEnd = purlEnd - 1;
		    }
			ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,p - pcur);
			char *pNewUrl;
			int nNewUrl;
			ChangeUrl(pctx->p, pctx, pconf, p,purlEnd - p, &pNewUrl, &nNewUrl);
			memstream_write(apwrite,pNewUrl,nNewUrl);
			ReplaceHTTPAndConvert(pctx,apwrite,pconf,purlEnd,pend - purlEnd + 1);
			return 1;
		}
		else{
			//如果找不到对应的结束符
			ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,p - pcur);
			int n, len = 0;
			char *purl = 0;
			n = UrlMapHTTP(pctx->p, pctx, pconf, p, pend-pcur + 1, &purl, &len);
			if (n > 0) {
				pcur = p + n;
				memstream_write(apwrite, purl, len);
				pcur = pcur + n;
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pcur,pend - pcur + 1);
				return 1;
			}
			else{
				memstream_write(apwrite, str, strlen(str));
				ReplaceHTTPAndConvert(pctx,apwrite,pconf,pdomain,pend - pdomain + 1);
				return 1;
			}

		}
	}
	translate(1,pconf,pcur,pend - pcur + 1,apwrite,pctx->p);
	return 1;
}

/*int ReplaceHTTPAndConvert_orig(ConvertCtx *pctx, memstream *apwrite, config *pconf, char *pbuf, int nsize)
{
	char *p;
	char *pend;
	char *pcur;
	
	int  i;
	char *str,*phttp,*phttps;
	
	pcur = pbuf;
	pend = pbuf + nsize - 1;
	
	if (pctx->nNotChangeTextboxUrl == 1)
	{
		convert(pconf->m_usetable, pcur,pend-pcur+1, apwrite, pconf,pctx);
		pctx->nNotChangeTextboxUrl = 0;
		return 1;
	}
	
	while(pcur <= pend)
	{
		if (pconf->m_iNotReplaceUrl != 1) {
			phttp=strnistr(pcur, "HTTP://", pend - pcur + 1);
			phttps=strnistr(pcur, "HTTPS://", pend - pcur + 1);
		}
		else {
			phttp = phttps = NULL;
		}
		
		if (phttp!=NULL || phttps!=NULL)
		{
			if (phttp==NULL)
			{
				p=phttps;
				str=pconf->m_pcSUrlPrefix;
				i=8;
			}
			else if (phttps==NULL)
			{
				p=phttp;
				str=pconf->m_pcUrlPrefix;
				i=7;
			}
			else if (phttp<phttps)
			{
				p=phttp;
				str=pconf->m_pcUrlPrefix;
				i=7;
			}
			else
			{
				p=phttps;
				str=pconf->m_pcSUrlPrefix;
				i=8;
			}
			
			convert(pconf->m_usetable, pcur, p - pcur, apwrite, pconf,pctx);
			{// WPF 2003-11-27
				int n, len = 0;
				char *purl = 0;
				n = UrlMapHTTP(pctx->p, pctx, pconf, p, pend - p +1, &purl, &len);
				if (n > 0) {
					pcur = p + n;
					memstream_write(apwrite, purl, len);
				}
				else {
					pcur=p+i;
					memstream_write(apwrite,str,strlen(str));
				}
			} 
			continue;
		}		
		else
		{
			convert(pconf->m_usetable, pcur,pend-pcur+1, apwrite, pconf,pctx);
			break;
		}
	}
	
	return 1;
}*/

char * GetLink(pool *apool,char *atrname, char *inbuf, int nsize)
{
	char *pend;
	char *pcur;
	char *p; 
	char *p1;
	char buf[2000];
	
	pcur = inbuf;
	pend = pcur + nsize - 1;
	
	p = strnistr(inbuf, atrname,nsize);
	if (!p)
		return NULL;
	p = strchr(p,'=');
	if (p)
		p++;
	else
		return NULL;
	while(*p==' ' || *p == '\t' || *p == '\r' || *p =='\n')
	{
		p++;
	}
	if (*p == '"' || *p == '\'')
	{
		p1 = strnchr(p+1,pend-p,*p);
		p++;
		if (!p1)
			p1 = pend;
	}	
	else
	{
		if (*p == '\\' && p < pend && *(p + 1) == '"') {
			return NULL;
		}
		
		p1 = strnchr(p, pend -p + 1, ' ');
		if (!p1)
			p1 = strnchr(p, pend-p+1,'\t');
		if (!p1)
			p1 = strnchr(p, pend-p+1,'\n');
		if (!p1)
			p1 = strnchr(p, pend-p+1,'\r');
		if (!p1)
			p1 = pend;
		/* p1 = pend + 1; WPF 10.10 2001 */
	}
	
	if ((p1 - p) < sizeof(buf) - 1)
	{
		memcpy(buf, p, p1-p);
	}
	else
	{
		return NULL;
	}
	
	{
		int  nnow=p1-p;
		char *ptrcur=strnistr(buf,"://",nnow);
		if (ptrcur)
		{
			ptrcur+=3;
			if (!strnchr(ptrcur, nnow + buf - ptrcur, '/'))
			{
				buf[nnow]='/';
				nnow++;
			}
		}
		buf[nnow]='\0';
	}		
	
	return apr_pstrdup(apool,buf);
}

/*
*This handles the embeded src javascript
*/
int if_http_plain_handler(request_rec *r, pool *apool, char *url, memstream *apwrite, config *pconfig)
{
	int port = 80, num;
	char cmd[2560];
	char page[2560];
	char host[2560];	
	char buff[4 << 10];
	char *pb, *ptr = NULL;
	int  size = 4 << 10;
	int s;
	struct hostent* phe;
	struct sockaddr_in sin;
	int flag = 0;
	memstream *unpms = create_memstream(apool);
	
	if ((ptr = strstr(url, "://")) == NULL) { 
		return -1;
	}
	ptr += 3;
	
	pb = strchr(ptr, '/'); 
	if (pb != NULL) {
		memcpy(host, ptr, pb - ptr);
		host[pb - ptr] = '\0';
		strcpy(cmd, pb);
	}
	else {
		strcpy(host, pb);
		strcpy(cmd, "/");
	}
	
	pb = strchr(host, ':');
	if (pb != NULL) {
		port = atoi(pb + 1);
		*pb = '\0';
	}
	
	sprintf(page, "GET %s HTTP/1.0\n\n", cmd);	
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	
	while (1) {
		if ((phe = gethostbyname(host)) == NULL) {
			break;
		}
		memset(&sin, 0, sizeof(sin));
		memcpy(&sin.sin_addr, phe->h_addr_list[0], phe->h_length);
		sin.sin_family = AF_INET;
		sin.sin_port = htons((unsigned short)port);	
		if (connect(s,(struct sockaddr*)&sin,sizeof(sin)) == -1) {
			break;
		}
		if (send(s, page, strlen(page), 0) < 0) {
			break;
		}
		
		do {
			num = recv(s, buff, size, 0);
			if (num > 0) {
				if (atoi(buff + 9) != 200) {
					ptr = NULL;
					break;
				}
			}
			else {
				ptr = NULL;
				break;
			}
			
			ptr = buff;
			while (ptr < buff + num) {
				if (!strncmp(ptr, "\r\n\r\n", 4)) {
					ptr += 4;
					break;
				}
				else 
					ptr++;
			}
			
			if (ptr == buff + num) { 
				ptr = NULL; 
			}
		}while (ptr == NULL);
		
		if (ptr == NULL) {
			break;
		}
		num = buff + num - ptr;		
		memstream_write(unpms, ptr, num);	
		
		while ((num = recv(s, buff, size, 0)) > 0) {
			memstream_write(unpms, buff, num);
		}
		
		{
			int nr = 0;
			int no = get_memstream_datasize(unpms);
			char *ptemp = apr_palloc(apool, LOGALLOCSIZE(no));			  
			char *unbuf = apr_palloc(apool, LOGALLOCSIZE(no));		
			nr = memstream_read(unpms, ptemp, no);
			if (nr > 0) {
				int k = 0;
				unsigned char code[] = {0xFF, 0xFE}; 
				unsigned char feff[] = {0xFE, 0xFF};
				if (!memcmp(ptemp, code, 2)) {
					k = ConvertFromUnicodeEx(pconfig->m_iToEncode, 0xFFFE, ptemp + 2, nr - 2, unbuf);
					memstream_write(apwrite, unbuf, k);					
				}
				else if (!memcmp(ptemp, feff, 2)) {
					k = ConvertFromUnicodeEx(pconfig->m_iToEncode, 0xFEFF, ptemp + 2, nr - 2, unbuf);
					memstream_write(apwrite, unbuf, k);					
				}
				else
					memstream_write(apwrite, ptemp, nr);
				
				flag = 1;
			}
		}
		break;
	}
	
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
	
	return flag;
}

int ProcessScript(pool *apool, ConvertCtx *apCtx, config *pconf, memstream *apwrite,char *pbuf, int nsize)
{
	char *ptage;
	char *pend;
	char *link,*newlink;
	int cbnewlink;
	char jshead[]="<script>";
	char jstail[]="</script>";
	
	pend = pbuf + nsize - 1;
	// ptage = strchr(pbuf,'>');
	ptage = strnistr(pbuf, ">", nsize);
	
	if (pconf->m_iShouldExpandJs == 1)
	{
		link = GetLink(apool,"SRC",pbuf,ptage-pbuf + 1);
		if (link)
		{
			if (pconf->m_iProcessScriptLevel > 0)
			{
				char *buf = (char*) apr_palloc(apool, strlen(link) + strlen(apCtx->m_pcCurrentUrl) + 128);
				strcpy(buf, link);
				if (apCtx->fjtignoreurl == 0) {
					strcat(buf, URL_DELIMITER);
					strcat(buf, URL_TYPE_JS);
					strcat(buf, apCtx->m_pcCurrentUrl);
					strcat(buf, "_.js");
				}
				if (ChangeUrl(apool, apCtx, pconf, buf, strlen(buf), &newlink, &cbnewlink) == 1)
					link = newlink;					
			}
			else
			{
				if (ChangeUrl(apool,apCtx,pconf,link, strlen(link), &newlink,&cbnewlink)==1)
					link = newlink;	
			}			
			memstream_write(apwrite,jshead,strlen(jshead));
			if_http_plain_handler(apCtx->r, apool, link, apwrite, pconf);
			memstream_write(apwrite,jstail,strlen(jstail));
			return 0;
		}
	}
	
	if (pconf->m_iProcessScriptLevel < 2)
	{
		if (ptage)
		{
			ProcessTag(apool, apCtx, pconf, pbuf, ptage-pbuf+1, apwrite);
		}
		ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptage+1, pend - ptage);
	}
	else
	{
		if (ptage)
		{
			ProcessTag(apool, apCtx, pconf, pbuf, ptage-pbuf+1, apwrite);
		}
		ChangeScript(apool, apCtx, apwrite, pconf, ptage+1, pend - ptage);
	}
	return 0;
}

int GetToken(char *pcIn, int nSize, char *pcOut, int nOutSize)
{
	int i;
	for (i=0; i<nSize; i++)
	{
		if (pcIn[i] == ' ' || pcIn[i] == '\t' || pcIn[i] == '\r' || pcIn[i] == '\n')
		{
			break;				
		}
		if (pcIn[i] == '"' || pcIn[i] == ',' || pcIn[i] == '.')
			return 0;
	}
	if (i==0)
		return 0;
	if (i<nOutSize)
	{
		memcpy(pcOut, pcIn, i);
		pcOut[i] = 0;
		return 1;
	}
	else
		return 0;
}

char* find_url_tail(char *url, char *end, char tail)
{
	int plus = 0;
	
	while (url < end) {
		if (*url == '+') {
			if (++plus > 1) {
				break;
			}
		}
		else if (*url == tail) {
			return url;
		}
		++url;
	}
	
	return NULL;
}

int ProcessTag(pool *apool, ConvertCtx *apCtx, config *pconf, char *ptagb, int nsize, memstream *apwrite)
{
	char *pcur, *pend;
	char *pnb, *pne;                 
	int  liNameLen;
	char cxAtrName[64];
	
	int isaction = 0;
	
	/* first find the tag name */
	if ((pconf->m_iProcessScriptLevel == 2)
		&& (strnistr(ptagb, "onMouseOut", nsize)!=NULL || strnistr(ptagb,"OnMouseOver", nsize)!=NULL || strnistr(ptagb, "OnClick", nsize) != NULL))
	{
		return ChangeScript(apool,apCtx,apwrite,pconf,ptagb,nsize);
	}
	pcur = ptagb+1;
	pend = ptagb + nsize - 1;
	/* Skip White Space */
	while(*pcur == ' ' || *pcur == '\t')
		pcur++;
	
	if (*pcur == '/')
	{
		int  nlen;
		char *pHead = pcur;
		char addcss[2048];
		
		while(*pHead == ' ' || *pHead == '\t')
			pHead++;
		
		if (strnistr(pHead, "head", 4) != NULL)
		{
			{/* Open HongKong words detect */
				apCtx->inbody = 1;
			}
			
			if ((*(pconf->m_pcInsertCSS) != '\0') && (strnicmp(pconf->m_pcInsertCSS, "none", 4) != 0))
			{
				strcpy(addcss, "<link rel=\"stylesheet\" href=\"");
				strcat(addcss, pconf->m_pcInsertCSS);
				strcat(addcss, "\" type=\"text/css\">\r\n");
				nlen = strlen(addcss);
				memcpy(addcss+nlen, ptagb, nsize);
				nlen += nsize;
				memstream_write(apwrite, addcss, nlen);
				return 1;
			}
		} 
		// WPF 2003-1-27 
		else if (pend - pHead > 11 && strnistr(pHead, "fjtignoreurl", 12) != NULL) { 
			apCtx->fjtignoreurl = 0; 
		}
		else if (pconf->m_iNotChangeTextboxUrl == 1 && pend - pHead >= 8 && strnistr(pHead, "textarea", 8) != NULL) { 
			apCtx->nNotChangeTextboxUrl = 0;
		} 
		
		ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);
		/*memstream_write(apwrite,ptagb,nsize);*/
		return 1;
	}
	
	pnb = pcur;
	while(pcur < pend)
	{
		if (*pcur == ' ' || *pcur == '\t' || *pcur=='\n' || *pcur=='\r' || *pcur == '>')
		{
			break;
		}
		pcur++;
	}
	if (*pcur == ' ' || *pcur == '\t' || *pcur=='\n' || *pcur=='\r' || *pcur == '>')
		pne = pcur - 1;
	else
		pne = pcur;
	
	liNameLen = pne - pnb + 1;
	
	{/* Process <?xml version="1.0" encoding="GB2312" ?> */
		if ((liNameLen == 4) && !strnicmp(pnb, "?xml", liNameLen))
		{
			char tempurl[2560];
			char *newptr = strnistr(ptagb, "encoding", nsize);
			
			if (newptr)
			{
				int ic = 0;
				char *codeptr;
				
				if ((codeptr = strnistr(newptr, "GB2312", ptagb + nsize - newptr)))
				{
					memcpy(tempurl, ptagb, codeptr - ptagb);
					if (pconf->m_iForceOutputUTF8 == 1 
						|| (pconf->m_iKeepUTF8Encode == 1 && apCtx->nIsUTF8 > 0)) {
						memcpy(tempurl + (codeptr - ptagb), "UTF-8", 5);					
						ic = 5;
					}
					else {
						memcpy(tempurl + (codeptr - ptagb), "BIG5", 4);					
						ic = 4;
					}
					memcpy(tempurl + (codeptr - ptagb) + ic, codeptr + 6, ptagb + nsize - (codeptr + 6));
					
					memstream_write(apwrite, tempurl, nsize - (6 - ic));
					return 1;
				}
				else if ((codeptr = strnistr(newptr, "GBK", ptagb + nsize - newptr)))
				{
					memcpy(tempurl, ptagb, codeptr - ptagb);
					if (pconf->m_iForceOutputUTF8 == 1 
						|| (pconf->m_iKeepUTF8Encode == 1 && apCtx->nIsUTF8 > 0)) {
						memcpy(tempurl + (codeptr - ptagb), "UTF-8", 5);					
						ic = 5;
					}
					else {
						memcpy(tempurl + (codeptr - ptagb), "BIG5", 4);					
						ic = 4;
					}
					memcpy(tempurl + (codeptr - ptagb) + ic, codeptr + 3, ptagb + nsize - (codeptr + 3));
					
					memstream_write(apwrite, tempurl, nsize - (3 - ic));
					return 1;
				}
				else if ((codeptr = strnistr(newptr, "BIG5", ptagb + nsize - newptr)))
				{
					memcpy(tempurl, ptagb, codeptr - ptagb);										
					if (pconf->m_iForceOutputUTF8 == 1 
						|| (pconf->m_iKeepUTF8Encode == 1 && apCtx->nIsUTF8 > 0)) {
						memcpy(tempurl + (codeptr - ptagb), "UTF-8", 5);	
						ic = 5;
					}
					else {
						memcpy(tempurl + (codeptr - ptagb), "GB2312", 6);
						ic = 6;
					}
					memcpy(tempurl + (codeptr - ptagb) + ic, codeptr + 4, ptagb + nsize - (codeptr + 4));
					
					memstream_write(apwrite, tempurl, nsize - (4 - ic));
					return 1;
				}
			}			
		}
	}/* End */
	
	if (strnicmp(pnb, "A", liNameLen) == 0 && liNameLen == 1)
	{
		if ((pconf->m_iProcessScriptLevel == 2)
			&& (strnistr(ptagb,"Javascript:", nsize)!=NULL))
		{
			return ChangeScript(apool,apCtx,apwrite,pconf,ptagb,nsize);
		}
		strcpy(cxAtrName, "href");
		
		if (pconf->m_iMarkUTF8Url > 1) {
			isaction = 1;
		}
		
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb, "link", liNameLen) == 0 && liNameLen == 4)
	{
		strcpy(cxAtrName, "href");
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb, "applet", liNameLen) == 0 && liNameLen == 6)
	{
		strcpy(cxAtrName, "codebase");
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb,"area",liNameLen) == 0 && liNameLen == 4)
	{
		strcpy(cxAtrName, "href");
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb, "META",liNameLen) == 0 && liNameLen == 4)
	{
		char *p, *p1, *p2;		
		
		if (pconf->m_iDetectUTF8BOM != 4 &&
			strnistr(ptagb, "UTF-8", nsize) && apCtx->m_istate == 0) {
			apCtx->nIsUTF8 = 1;
		}
		++apCtx->m_istate;
		
		if ((p=strnistr(ptagb, "charset", nsize-8)) == NULL)
		{
			strcpy(cxAtrName, "Url");
			goto DoModiUrlTag;
		}
		else
		{
			p1 = strchr(p, '=');
			if (!p1)
			{
				memstream_write(apwrite,ptagb,nsize);
				return 1;
			}
			else
			{
				p1++;
				while(*p1 == ' ' || *p1 == '\t' ) 
					p1++;
				if (*p1 == '\'' || *p1 == '"')
					p1++;
				p2 = p1;
				while(p1 < pend)
				{
					if (*p1 == ' ' || *p1 == '\t' || *p1 == '\'' || *p1 == '"' || *p1 =='\\')
					{
						break;
					}
					p1++;
				}
			}
			memstream_write(apwrite, ptagb, p2 - ptagb);
			if ((pconf->m_iKeepUTF8Encode == 1
				&& strnicmp(p2, "UTF", 3) == 0) 
				|| (pconf->m_iForceOutputUTF8 == 1) || pconf->m_iIsUTF8==1)
			{
				memstream_write(apwrite,"UTF-8", 5);
			}
			else if (pconf->m_iToEncode == ENCODE_GB2312)
			{

				if (pconf->m_iUnconvertOutput == 1)
					memstream_write(apwrite,"BIG5", 4);
				else
					memstream_write(apwrite,"GB2312", 6);
			}
			else if (pconf->m_iToEncode == ENCODE_BIG5)
			{
				if (pconf->m_iUnconvertOutput == 1)
					memstream_write(apwrite,"GB2312", 6);
				else
					memstream_write(apwrite,"BIG5", 4);
			}
//			ReplaceHTTPAndConvert(apCtx, apwrite, pconf, p1, pend-p1+1);
			memstream_write(apwrite, p1, pend-p1+1);
			return 1;
		}
	}
	else if (strnicmp(pnb, "Frame", liNameLen) == 0 && liNameLen == 5)
	{
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	}
	else if(strnicmp(pnb, "IFrame", liNameLen) == 0 && liNameLen == 6)
	{
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	}
	else if(strnicmp(pnb, "Form", liNameLen) == 0 && liNameLen == 4)
	{
		strcpy(cxAtrName, "Action");
		
		isaction = 1;
		
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb, "IMG", liNameLen) == 0 && liNameLen == 3)
	{
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb, "IMAGE", liNameLen) == 0 && liNameLen == 5)
	{
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	}
	else if(strnicmp(pnb, "input", liNameLen) == 0 && liNameLen == 5)
	{
		if (pconf->m_iValueChangeChineseLevel > 0
			&& nsize > 19 && strnistr(pnb, "input type=\"hidden\"", 19))
		{
			strcpy(cxAtrName, "Value");
			goto DoModiUrlTag;
		}
		{/* Not change url in textbox */
			if (pconf->m_iNotChangeTextboxUrl == 1)
				apCtx->nNotChangeTextboxUrl = 1;
		}
		if ((pconf->m_iProcessScriptLevel == 2) 
			&& (strnistr(ptagb, "OnClick", nsize) != NULL)) {
			return ChangeScript(apool, apCtx, apwrite, pconf, ptagb, nsize);
		}
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	}
	else if(strnicmp(pnb, "Script", liNameLen) == 0 && liNameLen == 6)
	{
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	}
	else if(liNameLen == 3 && strnicmp(pnb, "xml", liNameLen) == 0) {
		strcpy(cxAtrName, "SRC");
		goto DoModiUrlTag;
	} 
	else if (strnicmp(pnb, "embed", liNameLen) == 0 && liNameLen == 5)
	{
		strcpy(cxAtrName,"SRC");
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb,"param", liNameLen) == 0 && liNameLen == 5)
	{/* <param name="CharSet" value="big5"> */
		int len = nsize;
		char *ptr = NULL, *temp = ptagb;
		
		if ((ptr = strnistr(ptagb, "CharSet", nsize))) {
			if ((ptr = strnistr(ptr, "value", ptagb + nsize - ptr))) {
				int num = 0;
				char *rtr = NULL;
				if ((rtr = strnistr(ptr, "GB2312", ptagb + nsize - ptr))) {
					ptr = rtr;
					num = ptr - ptagb;
					temp = apr_palloc(apool, LOGALLOCSIZE(nsize + 8));
					memcpy(temp, ptagb, num);
					memcpy(temp + num, "BIG5", 4);
					memcpy(temp + num + 4, ptr + 6, nsize - num - 6);
					len -= 2;					
				}
				else if ((rtr = strnistr(ptr, "BIG5", ptagb + nsize - ptr))) {
					ptr = rtr;
					num = ptr - ptagb;
					temp = apr_palloc(apool, LOGALLOCSIZE(nsize + 8));
					memcpy(temp, ptagb, num);
					memcpy(temp + num, "GB2312", 6);
					memcpy(temp + num + 6, ptr + 4, nsize - num - 4);
					len += 2;
				}
			}
		}
		else if (strnistr(ptagb, "movie", nsize)) {
			strcpy(cxAtrName, "value");
			goto DoModiUrlTag;
		}
		
		return ChangeScript(apool, apCtx, apwrite, pconf, temp, len);
	}
	else if (strnicmp(pnb, "Body", liNameLen) == 0 && liNameLen == 4)
	{
		apCtx->inbody = 1;
		if (strnistr(ptagb,"OnLoad",nsize)!=NULL)
		{
			return ChangeScript(apool,apCtx,apwrite,pconf,ptagb,nsize);
		}
		strcpy(cxAtrName, "Background");
		goto DoModiUrlTag;
	}
	else if (strnicmp(pnb, "Option", liNameLen) == 0 && liNameLen == 6 && pconf->m_iValueChangeChineseLevel > 1)
	{
		strcpy(cxAtrName, "Value");
		goto DoModiUrlTag;
	} 
	// WPF 2003-1-27 
	else if (liNameLen == 12 && strnicmp(pnb, "fjtignoreurl", liNameLen) == 0) {
		apCtx->fjtignoreurl = 1;
	}
	else if (liNameLen == 3 && strnicmp(pnb, "!--", liNameLen) == 0) {
		ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);
		return 1;
	}
	else if (pconf->m_iNotChangeTextboxUrl == 1 && liNameLen == 8 && strnicmp(pnb, "textarea", liNameLen) == 0) {
		apCtx->nNotChangeTextboxUrl = 0xFFFF;
	}
	else 
	{
		strcpy(cxAtrName, "Background");
		goto DoModiUrlTag;
	}
	
DoModiUrlTag:
	{
		char *patrb;
		
		/* find href */
		if (!pconf->m_iShouldChangeUrlInServer)
		{
			if (pconf->m_iChangeScriptByDefault == 1)
				return ChangeScript(apool,apCtx,apwrite,pconf,ptagb,nsize);
			else
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
			return 1;
		}
		
		patrb = strnistr(pcur, cxAtrName, pend - pcur + 1);
		
		{// 2008-06-19 ��ֹ�� this.resourcesSrc ����� Src ����  Src, ����ǲ���ƥ�䡣
			if (patrb > pcur 
				&& *(patrb - 1) != ' ' 
				&& *(patrb - 1) != '\t'
				&& *(patrb - 1) != '\r'
				&& *(patrb - 1) != '\n'
				) {
				patrb = NULL;
			}
		}
		
		if (!patrb)
		{
			if (pconf->m_iChangeScriptByDefault == 1)
				return ChangeScript(apool,apCtx,apwrite,pconf,ptagb,nsize);
			else
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
			return 1;
		}
		else		/*we found href*/
		{
			char *patre;
			char *p;
			char *pNewUrl;
			int	  nNewUrl;
			p = patrb + strlen(cxAtrName);
			
			/* skip wp to find '=' */
			while(*p == ' ' || *p == '\t') p++;
			if (*p != '=')
			{
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
				/*memstream_write(apwrite, ptagb, nsize);*/
				return 1;
			}
			
			/* now find the value */
			p+=1;
			while(*p==' ' || *p=='\t' || *p=='\r' || *p=='\n')p++;
			
			if (p > pend || *(p + 1) == '+') 
			{
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
				/*memstream_write(apwrite, ptagb, nsize);*/
				return 1;
			}
			
			if (*p == '\"')
			{
				p++;
				// patre = strnchr(p, pend - p + 1, '"');
				patre = find_url_tail(p, pend + 1, '\"');
			}
			else if (*p == '\'')
			{
				p++;
				// patre = strnchr(p, pend - p + 1, '\'');
				patre = find_url_tail(p, pend + 1, '\'');
			}
			else if ((*p == '\\') && p < pend && (*(p + 1) == '\'' || *(p + 1) == '\"'))
			{
				if (p + 1 < pend && *(p + 1) == *(p + 2)) {
					patre = NULL;
				}
				else {
					char *ps = NULL;				
					p++;
					if (*p == '\'')
						ps = "\\\'";
					else
						ps = "\\\"";				
					p++;
					patre = strnistr(p, ps, pend - p + 1);
				}
			}
			else
			{
				int IsMeta = 0;
				char c;
				char *pdelim;
				
				pdelim = patrb;
				if (strnicmp(pnb,"meta",4)==0)
				{
					IsMeta = 1;
					for(pdelim=patrb; pdelim>ptagb; pdelim--)
					{
						if (*pdelim=='\'' || *pdelim=='"' || *pdelim=='=')
							break;
					}
				}
				
				if (*pdelim != '\'' && *pdelim!='"')
				{
					c=' ';
					pdelim = &c;
				}
				
				for(patre=p; patre <= pend; patre++)
				{
					if (*patre == ' ' || *patre== '\t' || *patre == '\r' || *patre=='\n' || *patre=='>' 
						|| (IsMeta==1 && *patre==*pdelim ) )
					{
						break;
					}
				}
				
				if (patre > pend)
					patre = NULL;
			}
			
			if (!patre)
			{
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
				/*memstream_write(apwrite, ptagb, nsize);*/
				return 1;
			}
			patre = patre - 1;
			/*
			if ((*patre=='"' || *patre=='\'') && (*(patre-1)!='\\'))
			patre--;
			*/
			
			if (pconf->m_iValueChangeChineseLevel > 0 
				&& strnicmp(cxAtrName, "Value", 5) == 0
				&& strnistr(p, "%", patre - p + 1) == NULL)
			{
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
				/*memstream_write(apwrite, ptagb, nsize);*/
				return 1;
			}
			
			if (strnicmp(pnb,"Option",6) == 0 ||
				(pconf->m_iValueChangeChineseLevel > 0 
				&& strnicmp(cxAtrName, "Value", 5) == 0))
			{// WPF 2003-9-23 
				int nval = 0;
				char *pvalue = NULL;
				
				if (strnistr(p, "./", patre - p + 1) || strnistr(p, "?", patre - p + 1) )
				{
					ChangeUrl(apool, apCtx, pconf, p, patre - p + 1, &pvalue, &nval);                        
				}
				else {
					pvalue = BuildBase64Block(apCtx,p,patre-p+1);                    
					nval = strlen(pvalue);
				}                
				memstream_write(apwrite,ptagb, p - ptagb);
				memstream_write(apwrite, pvalue, nval);
				memstream_write(apwrite, patre+1, pend - patre);
				return 1;
			}
			else if ((pconf->m_iProcessScriptLevel > 0) && (strnicmp(pnb,"Script",6)==0))
			{
				char *buf = (char*) apr_palloc(apool, (patre - p + 1) + strlen(apCtx->m_pcCurrentUrl) + 128);
				memcpy(buf, p, patre - p + 1);
				buf[patre - p + 1] = '\0';
				{// WPF 2004-8-18
					char *ptail = buf + (patre - p);
					while (ptail > buf) {
						if (*ptail == ' ' || *ptail == '\r' || *ptail == '\n' || *ptail == '\t') {
							*ptail = '\0';
							--ptail;
						}
						else {
							break;
						}
					}
				}
				/*if (apCtx->fjtignoreurl == 0) {
					strcat(buf,URL_DELIMITER);
					strcat(buf,URL_TYPE_JS);
					strcat(buf,apCtx->m_pcCurrentUrl);
					strcat(buf, "_.js");
				}*/
				if (!ChangeUrl(apool, apCtx, pconf, buf, strlen(buf), &pNewUrl, &nNewUrl))
				{
					ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);
//					memstream_write(apwrite, ptagb, nsize);
					return 1;
				}


				//2018-4-24,这段代码应该是没有用的。
				if (*(patre + 1) == '\'' || *(patre + 1) == '\"')
				{
					{
						int i = 0;
						table_entry *reqhdrs = NULL;
						array_header *reqhdrs_arr = NULL;					
						
						reqhdrs_arr = (array_header *) apr_table_elts(pconf->m_pUrlMapJS); 
						if (reqhdrs_arr != NULL) {
							reqhdrs = (table_entry *) reqhdrs_arr->elts; 
							for (i = 0; i < reqhdrs_arr->nelts; i++) {
								char *fromUrl = reqhdrs[i].key; 
								char *toUrl = reqhdrs[i].val; 								
								
								if (strnicmp(fromUrl, buf, strlen(fromUrl)) == 0) {							
									char *nurljs = (char*) apr_palloc(apool, nNewUrl + strlen(toUrl) + 32);
									
									memcpy(nurljs, pNewUrl, nNewUrl);
									pNewUrl = nurljs;
									nurljs += nNewUrl;
									*nurljs++ = *++patre;
									*nurljs++ = ' ';
									memcpy(nurljs, toUrl, strlen(toUrl));
									nurljs += strlen(toUrl);
									*nurljs++ = ' ';
									nNewUrl = nurljs - pNewUrl;
									break;
								}
							}
						}
					}					
				}
				//////////////////////////////////////////////////////////////////////////
			}
			else if (1)
			{ /* If is UTF-8 coding */
				if ((pconf->m_iMarkUTF8Url > 0) && (apCtx->nIsUTF8 > 0) && isaction)
				{
					int len = 0;
					char *temp = NULL;
					
					len = patre - p + 1;
					if (len < 0) {
						return 1;
					}
					temp = apr_palloc(apool, 
						LOGALLOCSIZE(len + strlen(apCtx->m_pcCurrentUrl) + 64));
					memcpy(temp, p, len);
					temp[len] = '\0';
					
					if ((len < 1) 
						&& (apCtx->m_pcCurrentUrl != NULL)) {
						char oldurl[2560];
						char *pend = NULL;
						
						if (strlen(apCtx->m_pcCurrentUrl) > sizeof(oldurl) - 128) {
							return 1;
						}
						
						strcpy(oldurl, apCtx->m_pcCurrentUrl);
						if ((pend = strchr(oldurl, '?')) != NULL 
							|| (pend = strchr(oldurl, '#')) != NULL 
							|| (pend = strchr(oldurl, ' ')) != NULL) {
							*pend = '\0';
						}
						strcat(temp, oldurl); 
						len = strlen(temp); 
					}                    
					{// WPF 2003-3-14 
						char *ptp = strstr(temp, "://");
						if ((ptp == NULL)
							|| (strchr(ptp + 3, '/') != NULL)) {
							strcat(temp, "@UTF-8");
							len += 6;
						}
					}
					
					if (!ChangeUrl(apool, apCtx, pconf, temp, len, &pNewUrl, &nNewUrl))
					{
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
						/*memstream_write(apwrite, ptagb, nsize);*/
						return 1;
					}
				}
				else if (!ChangeUrl(apool, apCtx, pconf, p, patre - p + 1, &pNewUrl, &nNewUrl))
				{
					ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);			
					/*memstream_write(apwrite, ptagb, nsize);*/
					return 1;
				}
			}
			/*
			else if (!ChangeUrl(apool, apCtx, pconf, p, patre - p + 1, &pNewUrl, &nNewUrl))
			{	
			ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, nsize);							
			return 1;
			}
			*/
			
			/*Now ChangedUrl succeeded*/
			{
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, p - ptagb);			
				/*memstream_write(apwrite, ptagb, p - ptagb);*/
				
				memstream_write(apwrite, pNewUrl, nNewUrl);
				ReplaceHTTPAndConvert(apCtx, apwrite, pconf, patre+1, pend - patre);			
				/*memstream_write(apwrite, patre+1, pend - patre);*/
				return 1;
			}
		}
		return 1;
	}	/* end of DoModiUrlTag */
	return 1;
}

int ParseHtml(pool *apool, ConvertCtx *apCtx, config *pconf, char *inbuf, int size, memstream *apwrite)
{
	int listate;
	char *pcur;			/*The current pos of the inbuf stream*/
	char *pend;			/*End of the Input Stream, when pcur==pend , the input is finished*/
	char *ptextb;		/*Begin of the current text block*/
	char *ptexte;		/*End of the current text block*/
	listate = 0; 
	
	/*
	WPF 2003-4-5
	CheckLeftBuffer(apCtx, pconf, &inbuf, &size);
	*/ 
	
	pcur = inbuf;
	pend = pcur + size - 1;
	if (size==0 || inbuf==NULL)
		return 0;
	while(pcur <= pend)
	{
		
		switch(listate)
		{
		case 0:		/*in the Begining of the text*/
			{
				ptextb = pcur;
find_again:
				ptexte = strnchr(pcur, pend - pcur + 1,'<');
				
				if (ptexte == NULL)
					ptexte = pend;
				else if (ptexte[1]==' ' || ptexte[1] == '\t' || (ptexte[1] >= '0' && ptexte[1] <= '9'))
				{
					pcur = ptexte + 1;
					goto find_again;
				}
				else
					ptexte = ptexte - 1;
				
				if (ptexte >= ptextb)
					if (pconf->m_iReplaceHttpInText != 1)
					{
						convert(pconf->m_usetable, 
							ptextb, 
							ptexte-ptextb+1,
							apwrite,
							pconf,apCtx);
					}
					else
					{
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf,ptextb, ptexte-ptextb+1);			
					}
					pcur = ptexte + 1;
					listate = 1;
					break;
			} /*case 0*/
			
		case 1:		/* we are in the tag */
			{
				/*find the match ">" */
				char *ptagb;
				char *ptage;
				char cxTagName[64];
				
				ptagb = pcur;
				/* 
				Because some pages have errors, like kimos, we have to deal with 
				no match situation
				*/
				
				/*
				ptage = FindMatch(pcur, pend - pcur + 1,'>');
				*/
				
				ptage = strnchr(pcur, pend - pcur + 1, '>');
				/* if no matching is found then this is actually text */
				
				
				if (ptage == NULL)
				{
					ptage = pend;
					
					convert(pconf->m_usetable, 
						ptagb, 
						ptage-ptagb+1,
						apwrite,
						pconf,apCtx);
					pcur = ptage + 1;
					break;
				}
				
				/* if no Token is found then this is actually text */
				if (!GetToken(ptagb+1, ptage-ptagb-1,cxTagName,sizeof(cxTagName)))
				{
					convert(pconf->m_usetable, 
						ptagb, 
						ptage-ptagb+1,
						apwrite,
						pconf,apCtx);
					pcur = ptage + 1;
					listate = 0;
					continue;
				}
				if (stricmp(cxTagName, "Script")==0)
				{
					char *p;
					p = strnistr(ptage,"</Script>", pend - ptage);
					if (p == 0)			/*if no matching token then */
					{
						convert(pconf->m_usetable,ptagb, pend-ptagb, apwrite, pconf,apCtx);
						pcur = pend; 
						listate = 0;
						continue;
						
					}
					else
					{
						ProcessScript(apool, apCtx,pconf,apwrite,ptagb,p+9 - ptagb);
						pcur = p+9;
						listate = 0;
						continue;
					}
				}
				else if(stricmp(cxTagName, "base") == 0) {
					char *p = NULL;
					if ((p = GetLink(apool, "href", ptagb, ptage - ptagb + 1))) {
						apCtx->m_pcCurrentUrl = apr_pstrdup(apool, p);
					}
					ProcessTag(apool, apCtx, pconf, ptagb, ptage - ptagb + 1, apwrite);
				}
				else if (stricmp(cxTagName,"body")==0)
				{
					
					ProcessTag(apool, apCtx, pconf, ptagb, ptage-ptagb+1, apwrite);
				}
				else
				{
					
					ProcessTag(apool, apCtx, pconf, ptagb, ptage-ptagb+1, apwrite);
				}
				pcur = ptage+1;
				listate = 0;
				
			}/*case 1*/
		}
	}
	return 1;
}

int ParseHtml_last(pool *apool,ConvertCtx *apCtx,config *pconf,char *inbuff,int sizee,memstream *apwrite)
{ 
	int  listate = 0;
	char *pcur = NULL, *pend = NULL; 
	char *ptextb = NULL, *ptexte = NULL;
	
	int  no = 0, size = 0;
	char *ptr = NULL, *inbuf = NULL;
	
	if (sizee==0 || inbuff==NULL)
		return 0;
	
	no = apCtx->bakno;
	size = sizee + no; 
	if (no)
	{ 
		int au = 0;
		char *pau = NULL;
		
		if (apCtx->n_BakBuf < size) {
			au = apCtx->n_BakBuf;
			pau = apCtx->p_BakBuf;
			if (size < 8192) {
				apCtx->n_BakBuf = 8192;
			}
			else {
				apCtx->n_BakBuf = size;
			}
			apCtx->p_BakBuf = apr_palloc(apCtx->p, LOGALLOCSIZE(apCtx->n_BakBuf)); 
		} 
		
		inbuf = apCtx->p_BakBuf;
		memcpy(inbuf,apCtx->bakinfo,no);
		memcpy(inbuf+no,inbuff,sizee);				
		apCtx->bakno=0;
		
		if (au > apCtx->nbakinfo) {
			apCtx->nbakinfo = au;
			apCtx->bakinfo = pau; 
		} 
	}
	else
		inbuf=inbuff; 
	
	pcur=inbuf;
	pend=pcur+size-1;
	
	while(pcur<=pend)
	{
		if (pcur==pend)
		{
			ReplaceHTTPAndConvert(apCtx, apwrite, pconf, pcur, 1);
			break;
		}
		
		if ((*pcur=='<') && (*(pcur+1)!=' ') && (*(pcur+1)!='\t') && ((*(pcur+1)<'0') || (*(pcur+1)>'9')))
			listate=1;
		else 
			listate=0;
		
		switch(listate)
		{
		case 0:		
			{
				ptextb=pcur;
				ptr=ptextb;
				
				while(ptr<pend)
				{
					if ((*ptr=='<') && (*(ptr+1)!=' ') && (*(ptr+1)!='\t') && ((*(ptr+1)<'0') || (*(ptr+1)>'9')))
						break;
					if ((*ptr & 0x80)!=0)
					{
						ptr+=2;
						continue;
					}
					ptr++;
				}
				
				if (ptr!=pend)
				{
					ptexte=ptr-1;	
					
					if (pconf->m_iReplaceHttpInText != 1)
					{
						convert(pconf->m_usetable, 
							ptextb, 
							ptexte-ptextb+1,
							apwrite,
							pconf,apCtx);
					}
					else
					{
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf,ptextb, ptexte-ptextb+1);
					}
					pcur=ptr;						
					break;
				}
				else
				{								
					if ((*ptr=='<') || (*ptr & 0x80)!=0)
					{
						ptexte=ptr-1;
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptr, 1);
					}
					else
						ptexte=ptr;					
					
					if (pconf->m_iReplaceHttpInText != 1)
					{
						convert(pconf->m_usetable, 
							ptextb, 
							ptexte-ptextb+1,
							apwrite,
							pconf,apCtx);
					}
					else
					{
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf,ptextb, ptexte-ptextb+1);
					}
					
					
					pcur=pend+1;
					break;				
				}
				
			}
			
		case 1:	
			{				
				char *ptagb;
				char *ptage;
				char cxTagName[32];
				
				ptagb=pcur;
				ptr=ptagb;
				
				while(ptr<=pend)
				{					
					if ((*ptr & 0x80)!=0)
					{
						ptr+=2;
						continue;
					}
					if (*ptr=='>') 
						break;
					ptr++;
				}
				
				if (ptr<=pend)
				{
					ptage=ptr;
					
					if (!GetToken(ptagb+1,ptage-ptagb-1,cxTagName,sizeof(cxTagName)))
					{
						convert(pconf->m_usetable,ptagb,ptage-ptagb+1,apwrite,pconf,apCtx);
						pcur=ptage+1;
						break;
					}
					
					if (strnicmp(cxTagName,"Script", 6)==0
						&& pconf->m_iTreatScriptAsHtml != 1)
					{
						char *p;					
						p=strnistr(ptage,"</Script>",pend-ptage);
						if (p==0)			
						{ 
							ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, pend-ptagb+1);
							pcur=pend+1;					
							break;
						}
						else 
						{
							ProcessScript(apool,apCtx,pconf,apwrite,ptagb,p+9-ptagb);
							pcur=p+9;							
							break;
						}
					}
					else if(stricmp(cxTagName, "base") == 0) {
						char *p = NULL;
						if ((p = GetLink(apool, "href", ptagb, ptage - ptagb + 1))) {
							apCtx->m_pcCurrentUrl = apr_pstrdup(apool, p);
						}
						ProcessTag(apool, apCtx, pconf, ptagb, ptage - ptagb + 1, apwrite);
					}
					else
					{	
						ProcessTag(apool,apCtx,pconf,ptagb,ptage-ptagb+1,apwrite);
					}
					pcur=ptage+1;					
					break;
				}
				else 
				{ 
					ReplaceHTTPAndConvert(apCtx, apwrite, pconf, ptagb, pend-ptagb+1);
					pcur=pend+1;					
					break;
				}
				
			}
		}
		
	}
	
	return 1;
} 

int ParseHtml_html(pool *apool,ConvertCtx *apCtx,config *pconf,char *inbuff,int sizee,memstream *apwrite)
{ 
	int  listate = 0;
	char *pcur = NULL, *pend = NULL; 
	char *ptextb = NULL, *ptexte = NULL;
	
	int  no = 0, size = 0;
	char *ptr = NULL, *inbuf = NULL;
	
	if (sizee==0 || inbuff==NULL)
		return 0;
	
	no = apCtx->bakno;
	size = sizee + no; 
	if (no)
	{ 
		int au = 0;
		char *pau = NULL;
		
		if (apCtx->n_BakBuf < size) {
			au = apCtx->n_BakBuf;
			pau = apCtx->p_BakBuf;
			if (size < 8192) {
				apCtx->n_BakBuf = 8192;
			}
			else {
				apCtx->n_BakBuf = size;
			}
			apCtx->p_BakBuf = apr_palloc(apCtx->p, LOGALLOCSIZE(apCtx->n_BakBuf)); 
		} 
		
		inbuf = apCtx->p_BakBuf;
		memcpy(inbuf,apCtx->bakinfo,no);
		memcpy(inbuf+no,inbuff,sizee);				
		apCtx->bakno=0;
		
		if (au > apCtx->nbakinfo) {
			apCtx->nbakinfo = au;
			apCtx->bakinfo = pau; 
		} 
	}
	else
		inbuf=inbuff; 
	
	pcur=inbuf;
	pend=pcur+size-1;
	
	while(pcur<=pend)
	{
		if (pcur==pend)
		{
			if (apCtx->nbakinfo < 1)
			{
				apCtx->bakinfo = apr_palloc(apCtx->p, LOGALLOCSIZE(1024));
				apCtx->nbakinfo = 1024;
			}
			memcpy(apCtx->bakinfo,pcur,1);
			apCtx->bakno=1;
			break;
		}
		
		if ((*pcur=='<') && (*(pcur+1)!=' ') && (*(pcur+1)!='\t') && ((*(pcur+1)<'0') || (*(pcur+1)>'9')))
			listate=1;
		else 
			listate=0;
		
		switch(listate)
		{
		case 0:		
			{
				ptextb=pcur;
				ptr=ptextb;
				
				while(ptr<pend)
				{
					if ((*ptr=='<') && (*(ptr+1)!=' ') && (*(ptr+1)!='\t') && ((*(ptr+1)<'0') || (*(ptr+1)>'9')))
						break;
					if ((*ptr & 0x80)!=0)
					{
						ptr+=2;
						continue;
					}
					ptr++;
				}
				
				if (ptr!=pend)
				{
					ptexte=ptr-1;	
					
					if (pconf->m_iReplaceHttpInText != 1)
					{
						/*convert(pconf->m_usetable,
							ptextb, 
							ptexte-ptextb+1,
							apwrite,
							pconf,apCtx);*/
						translate(1,pconf,ptextb,ptexte-ptextb+1,apwrite,apCtx->p);
					}
					else
					{
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf,ptextb, ptexte-ptextb+1);
					}
					pcur=ptr;						
					break;
				}
				else
				{								
					if ((*ptr=='<') || (*ptr & 0x80)!=0)
					{
						ptexte=ptr-1;
						if (apCtx->nbakinfo < 1)
						{
							apCtx->bakinfo = apr_palloc(apCtx->p, LOGALLOCSIZE(4096));
							apCtx->nbakinfo = 4096;
						}
						apCtx->bakinfo[0]=*ptr;
						apCtx->bakno=1;
					}
					else
						ptexte=ptr;					
					
					if (pconf->m_iReplaceHttpInText != 1)
					{
						translate(1,pconf,ptextb,ptexte-ptextb+1,apwrite,apCtx->p);
					}
					else
					{
						ReplaceHTTPAndConvert(apCtx, apwrite, pconf,ptextb, ptexte-ptextb+1);
					}
					
					
					pcur=pend+1;
					break;				
				}
				
			}
			
		case 1:	
			{				
				char *ptagb;
				char *ptage;
				char cxTagName[32];
				
				ptagb=pcur;
				ptr=ptagb;
				
				while(ptr<=pend)
				{					
					if ((*ptr & 0x80)!=0)
					{
						ptr+=2;
						continue;
					}
					if (*ptr=='>') 
						break;
					ptr++;
				}
				
				if (ptr<=pend)
				{
					ptage=ptr;
					
					if (!GetToken(ptagb+1,ptage-ptagb-1,cxTagName,sizeof(cxTagName)))
					{
//						convert(pconf->m_usetable,ptagb,ptage-ptagb+1,apwrite,pconf,apCtx);
						translate(1,pconf,ptextb,ptexte-ptextb+1,apwrite,apCtx->p);
						pcur=ptage+1;
						break;
					}
					
					if (strnicmp(cxTagName,"Script", 6)==0
						&& pconf->m_iTreatScriptAsHtml != 1)
					{
						char *p;					
						p=strnistr(ptage,"</Script>",pend-ptage);
						if (p==0)			
						{ 
							int na = pend - ptagb + 1;
							
							if (apCtx->nbakinfo < na) {
								if (na < 4096) {
									apCtx->nbakinfo = 4096;
								}
								else {
									apCtx->nbakinfo = na;
								}
								apCtx->bakinfo = apr_palloc(apCtx->p, LOGALLOCSIZE(apCtx->nbakinfo));
							} 
							memcpy(apCtx->bakinfo,ptagb,pend-ptagb+1);
							apCtx->bakno=pend-ptagb+1;
							pcur=pend+1;					
							break;
						}
						else 
						{
							ProcessScript(apool,apCtx,pconf,apwrite,ptagb,p+9-ptagb);
							pcur=p+9;							
							break;
						}
					}
					else if(stricmp(cxTagName, "base") == 0) {
						char *p = NULL;
						if ((p = GetLink(apool, "href", ptagb, ptage - ptagb + 1))) {
							apCtx->m_pcCurrentUrl = apr_pstrdup(apool, p);
						}
						ProcessTag(apool, apCtx, pconf, ptagb, ptage - ptagb + 1, apwrite);
					}
					else
					{	
						ProcessTag(apool,apCtx,pconf,ptagb,ptage-ptagb+1,apwrite);
					}
					pcur=ptage+1;					
					break;
				}
				else 
				{ 
					int nc = pend - ptagb + 1;
					if (apCtx->nbakinfo < nc) {
						if (nc < 1024) {
							apCtx->nbakinfo = 1024;
						}
						else {
							apCtx->nbakinfo = nc;
						}
						apCtx->bakinfo = apr_palloc(apCtx->p, LOGALLOCSIZE(apCtx->nbakinfo));
					} 
					memcpy(apCtx->bakinfo,ptagb,pend-ptagb+1);
					apCtx->bakno=pend-ptagb+1;
					pcur=pend+1;					
					break;
				}
				
			}
		}
		
	}
	
	return 1;
} 

/*
char* url_browser(request_rec *r, pool *apool, config *pconf, ConvertCtx *pctx, char *url, int *nsize)
{
int len;
char *pout, temp[2560];
char *psplit, *ptr = temp;

  psplit = strchr(url, '?');
  if (!psplit)
		return url;
		
		  psplit++;	
		  UrlDecode(psplit, strlen(psplit), temp, &len);
		  
			while (ptr < temp + len)
			{
			if (*ptr & 0x80)
			break;
			else
			ptr++;
			}
			
			  if (ptr < temp + len)
			  {
			  int nlan = -1;
			  char *plang = (char*) apr_table_get(r->headers_in, "ACCEPT-LANGUAGE");
			  
				if (!strcmp(plang, "zh-cn"))
				nlan = 0;
				else if (!strcmp(plang, "zh-hk") || !strcmp(plang, "zh-tw"))
				nlan = 1;
				
				  if (nlan == pconf->m_iFromEncode)
				  {
				  int nout;
				  config excon;
				  memstream *pstream = create_memstream(apool);
				  
					memcpy(&excon, pconf, sizeof(config));
					excon.m_iConvertWord = 0;
					
					  nout = convert(excon.m_usetable, temp, len, pstream, &excon, pctx);
					  nout = memstream_read(pstream, temp, sizeof(temp));
					  temp[nout] = '\0';
					  len = strlen(temp);
					  pout = apr_pcalloc(apool, LOGALLOCSIZE(2048));
					  UrlEncode(temp, len, pout, nsize);
					  *(pout + *nsize) = '\0';
					  len = strlen(pout) + 1;
					  nout = psplit - url;
					  memmove(pout + nout, pout, len);
					  memcpy(pout, url, nout);
					  
						return pout;
						}
						}
						
						  return url;
						  } 
						  */
						  
						  int ProcessEnt(ConvertCtx *pctx, config *pconfig, memstream *pstream, char *pboundary, char *pentb, char *pente)
						  {
							  int flag = 1;
							  char *p = pentb;
							  
							  if (pconfig->m_iNotDetectAttachment != 1) {
								  /* get the first line of the ent*/	
								  while(*p != '\r' && *p != '\n') {
									  p++;
								  }	
								  if (strnistr(pentb, "filename=", p - pentb)) { 
									  while (*p == '\r' || *p == '\n') {
										  p++;
									  }
									  {
										  char *p2 = p;
										  while (*p2 != '\r' && *p2 != '\n') {
											  p2++;
										  }
										  if (strnistr(p, "Content-Type:", p2 - p) && strnistr(p, "text/plain", p2 - p)) {
											  unconvert(pconfig->m_usetable, pentb, p - pentb, pstream, pconfig, pctx);
											  pentb = p;
										  }
										  else if (pconfig->m_iInShouldConvert != 0) {
											  unconvert(pconfig->m_usetable, pentb, p - pentb, pstream, pconfig, pctx);
											  pentb = p;
											  flag = 0;
										  }
										  /////////////
										  flag = 0;
									  }		
								  }
							  }
							  
							  if (flag == 1) 
							  {/* we should convert */		
								  if (pconfig->m_iInShouldConvert != 0)
								  {
									  unconvert(pconfig->m_usetable, pentb, pente-pentb, pstream, pconfig, pctx);			
								  }
								  else
								  {
									  memstream_write(pstream,pentb, pente-pentb);
								  }
							  }
							  else 
							  {/* we should not convert */
								  memstream_write(pstream,pentb, pente-pentb);
							  }
							  
							  return 1;
						  } 
						  
						  int IsDomainValid(table *tb, char *host)
						  {
							  array_header *reqhdrs_arr;
							  table_entry *reqhdrs;
							  int i;
							  int hostlen = strlen(host);
							  
							  reqhdrs_arr = (array_header*) apr_table_elts(tb);
							  reqhdrs = (table_entry*) reqhdrs_arr->elts;
							  
							  for (i = 0; i < reqhdrs_arr->nelts; i++) {
								  char *key = reqhdrs[i].key; 
								  int nk = strlen(key);
								  
								  if (nk > 1 && *(key + nk - 1) == '*') {
									  if (hostlen >= nk - 1 && strnicmp(key, host, nk - 1) == 0) {
										  return 1;
									  }
									  continue;
								  }
								  
								  if (*key=='.') {
									  if (hostlen >= nk && strnicmp(key, host + (hostlen - nk), nk) == 0) {
										  return 1;
									  }
								  }
								  else {
									  if (hostlen == nk && strnicmp(key, host, hostlen) == 0) {
										  return 1;
									  }
									  if (*key == '*' && nk == 1) {
										  return 1;
									  }
								  }
							  }
							  
							  return 0;
						  }
						  
						  void RedirectTip(request_rec *r, char *url, char *tips, config *pconfig)
						  {
							  char t0[] = "<HTML>\r\n<HEAD>\r\n<META HTTP-EQUIV=\"REFRESH\" CONTENT=\"3; URL=";
							  char t1[1024];
							  char t2[] = "\">";
							  char t3[] = "\r\n</HEAD>\r\n<BODY>\r\n<BR>\r\n<CENTER>";
							  char t4[] = "Will redirect to a new URL ";
							  char t5[] = "<A HREF = \"";
							  char t6[] = "</A></CENTER>\r\n</BODY>\r\n</HTML>";	
							  char *t = t4;
							  
							  { // ��ֹ��վ�ű����� <script>...</script> , �� <> ���б��롣
								  url = url_reset_prefix(url, "<", "%3C", r->pool);
								  url = url_reset_prefix(url, ">", "%3E", r->pool);
							  }
							  
							  if (tips && strlen(tips)) {
								  t = tips;
							  }	
							  strcpy(t1, t0);
							  if (pconfig->m_iRedirectTipTime > 0) {
								  itoa(pconfig->m_iRedirectTipTime, &t1[strlen(t0) - 7], 10);
								  strcat(t1, "; URL=");
							  }	
							  r->content_type = "text/html";
							  ap_rvputs(r, t1, url, t2, NULL);
							  ap_rvputs(r, t3, t, t5, url, t2, url, t6, NULL);
						  }
						  
						  int Redirect(request_rec *r, char *url, config *pconfig, char *msg)
						  {
							  char *p;
							  
							  if (pconfig)
							  {
								  p = UnChangeChinese(r->pool,pconfig, NULL, url);
								  
								  if (pconfig->m_iRedirectTip == 1)
								  {
									  char *ptr = p + strlen(p);
									  
									  if (*(ptr - 1) == '/' || !strnicmp(ptr - 4, ".htm", 4)
										  || !strnicmp(ptr - 5, ".html", 5) || !strnicmp(ptr - 4, ".asp", 4)
										  || !strnicmp(ptr - 4, ".jsp", 4) || !strnicmp(ptr - 4, ".php", 4))
									  {
										  RedirectTip(r, p, pconfig->m_pcRedirectTipString, pconfig);
										  return 0;
									  }
								  }
							  }
							  else
								  p = url;
							  
							  ap_rvputs(r, "HTTP/1.0 ", "302 Moved", CRLF, NULL);
							  ap_rvputs(r, "Location: ", p, CRLF, CRLF, NULL);
							  ap_rvputs(r, msg,NULL);	
							  ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "%s",msg);
							  
							  return 0;
						  }
						  
						  int lookcopyright(request_rec *r, config *pconfig, char *url)
						  {
							  extern char heaventest[128], lastdefense[128], gotourl[256], thetitle[1024];
							  int len = strlen(url);
							  
							  if (strnistr(url, heaventest, len) || strnistr(url, lastdefense, len))
							  {
								  Redirect(r, gotourl, pconfig, thetitle);
								  return 1;
							  }
							  
							  return 0;
						  } 
						  
						  int fnMIMEtype(svr_config *conf, char *pType, char *pUrl)
						  {// WPF 2002-4-30 
							  if (pType)
							  {
								  char *mime[] = {"application/zip", 
									  // "application/octet-stream",
									  "image/x-icon",
									  "application/x-compress",
									  "image/pjpeg",
									  "image/jpeg",
									  "image/gif",
									  "image/bmp",
									  "image/jpg",
									  "application/x-shockwave-flash", // WPF 2002-5-8
									  "application/pdf"}; 
								  
								  int i, num = sizeof(mime) / sizeof(mime[0]);
								  int len = strlen(pType);
								  
								  for (i = 0; i < num; i++)
								  {
									  if (strnistr(pType, mime[i], len))
										  return 1;
								  } 
								  
								  if (conf->m_iBinaryFileExt > 0) {
									  int i = 0;
									  while (i < conf->m_iBinaryFileExt) {
										  int len = strlen(conf->m_pcBinaryFileExt[i]);
										  if (strnicmp(conf->m_pcBinaryFileExt[i], pType, len) == 0) {
											  return 1;
										  }
										  i++;
									  }
								  }
								  if (conf->m_iHtmlFileExt > 0) {
									  int i = 0;
									  while (i < conf->m_iHtmlFileExt) {
										  int len = strlen(conf->m_pcHtmlFileExt[i]);
										  if (strnicmp(conf->m_pcHtmlFileExt[i], pType, len) == 0) {
											  return -1;
										  }
										  i++;
									  }
								  }
							  }
							  
							  if (pUrl)
							  {
								  char *ext[] = {".jar",
									  ".ico",
									  ".zip",			
									  ".swf",
									  ".png",
									  ".bmp",
									  ".pdf",
									  ".class",
									  ".cab",
									  ".exe"};
								  
								  int i, num = sizeof(ext) / sizeof(ext[0]);
								  int len = strlen(pUrl);
								  char *pend = pUrl + len;
								  
								  if (pType == NULL) {
									  num--;
								  }
								  
								  for (i = 0; i < num; i++)
								  {
									  int ne = strlen(ext[i]);
									  char *pcmp = pend - ne; 
									  
									  if (strnistr(pcmp, ext[i], ne))
										  return 1;
								  } 
								  
								  if (conf->m_iBinaryFileExt > 0) { 
									  int i = 0;
									  char *p = NULL;
									  p = strchr(pUrl, '?');
									  if (p == NULL) {
										  p = pend;
									  }
									  while (i < conf->m_iBinaryFileExt) {
										  int len = strlen(conf->m_pcBinaryFileExt[i]);
										  if (strnicmp(conf->m_pcBinaryFileExt[i], p - len, len) == 0) {
											  return 1;
										  }
										  i++;
									  }
								  }
								  if (conf->m_iHtmlFileExt > 0) { 
									  int i = 0;
									  char *p = NULL;
									  p = strchr(pUrl, '?');
									  if (p == NULL) {
										  p = pend;
									  }
									  while (i < conf->m_iHtmlFileExt) {
										  int len = strlen(conf->m_pcHtmlFileExt[i]);
										  if (strnicmp(conf->m_pcHtmlFileExt[i], p - len, len) == 0) {
											  return -1;
										  }
										  i++;
									  }
								  }    
							  }
							  
							  return 0;
}

int DetermineFileType(svr_config *conf, config *pconfig, char *datestr, char *url)
{
	if (pconfig->m_iNotConvertPage == 1) {
		return FILETYPE_AUTO_DETECT;
	}
	
	if (strstr(url, "@OldUrl;@JS;") != NULL) {
		return FILETYPE_JS;
	}
	
	{// WPF do for cgi .exe return html page 
		if (pconfig->m_iForceConvertPage == 1) {
			return FILETYPE_HTML;
		}
		else if (fnMIMEtype(conf, datestr, url) == 1) {
			return FILETYPE_AUTO_DETECT;
		}
		else if (fnMIMEtype(conf, datestr, url) == -1) {
			return FILETYPE_HTML;
		}
	}
	
	if (datestr != NULL)
	{
		int len_str = strlen(datestr);
		
		if (strnistr(datestr, "x-javascript", len_str) != NULL)
		{
			return FILETYPE_JS;	
		}
		else if (strnistr(datestr, "text/html", len_str)!=NULL)
		{
			return FILETYPE_HTML;
		}
		else if (strnistr(datestr, "text/xml", len_str)!=NULL)
		{
			return FILETYPE_HTML;
		}
		else if(strnistr(datestr, "text", len_str)!=NULL)
		{
			return FILETYPE_TEXT;
		}
	}
	
	if (url != NULL) {
		int l = strlen(url);		
		char *pend = url + l;
		
		if (strnistr(pend - 3, ".js", 3) != NULL) {
			return FILETYPE_JS;
		}
		else if (strnistr(pend - 4, ".jpg", 4)!=NULL 
			|| strnistr(pend - 4, ".gif", 4)!=NULL || strnistr(pend - 5, ".jpeg", 5)!=NULL )
		{
			return FILETYPE_AUTO_DETECT;
		}
		else if (strnistr(pend - 4, ".swf", 4)!=NULL || strnistr(pend - 4, ".cab", 4)!=NULL)
		{
			return FILETYPE_AUTO_DETECT; 
		}
		else {
			return FILETYPE_HTML;
		}
	}
	
	return FILETYPE_AUTO_DETECT;
}



char *ap_proxy_cookie_domain(pool *pool, ConvertCtx *pctx, config *pconfig, char *cookies)
{
	char path[1024] = {0};
	char domain[1024] = {0};	
	char expires[1024] = {0};
	char *u, *p, *p1;
	
	if (pconfig->m_iCookiePathMode > 0 
		&& pconfig->m_pcCookieDomain[0] != 0)
	{
		int nlen = 0;
		char *phead = strchr(cookies, ';');
		
		if (phead)
		{
			nlen = strlen(phead);
			
			p = strnistr(phead, "domain=", nlen);
			if (p)
			{
				p1 = strchr(p, ';');
				if (p1 == NULL)
					p1 = phead + nlen;
				p+=7;
				memcpy(domain, p, p1 - p);
				domain[p1 - p] = 0;
			}
			
			p = strnistr(phead, "path=", nlen);
			if (p)
			{
				p1 = strchr(p, ';');
				if (p1 == NULL)
					p1 = phead + nlen;
				p+=5;
				memcpy(path, p, p1 - p);
				path[p1-p] = 0;
			}
			
			p = strnistr(phead, "expires=", nlen);
			if (p)
			{
				p1 = strchr(p, ';');
				if (p1 == NULL)
					p1 = phead + nlen;
				p+=8;
				memcpy(expires, p, p1 - p);
				expires[p1-p] = 0;
			}
		}
		
		p = phead;
		if (!p)
		{
			if (pconfig->m_iCookiePathMode == 2)
			{
				u = apr_palloc(pool, LOGALLOCSIZE(strlen(cookies) + 128));								
				strcpy(u, cookies);
				strcat(u, ";path=/");				
				cookies = u;
				goto last;
			}
			else
			{
				goto last;
			}
		} 
		
		u = apr_palloc(pool, LOGALLOCSIZE(strlen(cookies) * 2 + 64));
		*p = '\0'; 
		
		{// WPF Process Cookie  
			while (pconfig->m_iCookiePathMode == 3) { 
				char *pt = NULL;
				char *pn = strchr(cookies, '=');
				
				if (pn == NULL) { 						
					break;
				} 
				if (domain[0] == '\0') {
					char *pb = NULL, *pe = NULL; 
					char *urd = pctx->m_pcCurrentUrl;
					
					if ((pb = strstr(urd, "://")) == NULL) {
						break;
					}
					pb += 3;
					pe = strchr(pb, '/');
					if (pe == NULL) { 
						strcpy(domain, pb);
					}
					else { 
						memcpy(domain, pb, pe - pb);
						domain[pe - pb] = '\0';
					}
				}
				
				memcpy(u, cookies, pn - cookies);
				*(u + (pn - cookies)) = '\0';
				
				strcat(u, "$D$"); 
				pt = trimstr(domain);
				strcat(u, pt);
				strcat(u, "#");
				
				if (path[0] != '\0') {
					strcat(u, "$P$");
					pt = trimstr(path);
					strcat(u, pt);
					strcat(u, "#");
				} 
				strcat(u, pn);
				
				if (expires[0] != '\0') { 
					strcat(u, ";expires=");
					strcat(u, expires);
				}
				if (domain[0] != '\0') { 
					strcat(u, ";domain=");
					strcat(u, pconfig->m_pcCookieDomain);
				}
				if (path[0] != '\0') {
					strcat(u, ";path=/");
				}
				cookies = u;
				goto last;
			}
		} 
		
		strcpy(u, cookies);
		
		if (expires[0])
		{
			strcat(u,";expires=");
			strcat(u,expires);
		}
		if(domain[0])
		{
			strcat(u,";domain=");
			strcat(u,pconfig->m_pcCookieDomain);
		}
		
		if (pconfig->m_iCookiePathMode == 1)
		{
			char *pnewpath;
			int nnewpath;
			
			ChangeUrl(pool,pctx,pconfig,path,strlen(path),&pnewpath,&nnewpath);
			pnewpath[nnewpath]=0;
			strcat(u,";path=");
			strcat(u, GetPathOfAbsUrl(pool,pnewpath));
		}
		else
		{
			strcat(u,";path=/");
		}
		cookies = u;
	}
	
last:
	// WPF 2005-7-12 
	if (pconfig->m_iConvertCookie == 1) 
	{
		int num = strlen(cookies);
		char *pb = NULL;
		memstream *tempmem = create_memstream(pctx->p);
		num = convert(pconfig->m_usetable, cookies, num, tempmem, pconfig, pctx);
		pb = apr_palloc(pctx->p, LOGALLOCSIZE(num + 8));
		num = memstream_read(tempmem, pb, num);
		*(pb + num) = '\0';
		cookies = pb;
	}
	return cookies;
} 

char* ResetUrl7E(char *url)
{
	char *ptr = url;
	char *pbk = url;	
	char *ped = NULL;
	
	if (strstr(url, "ifbase") == NULL
		&& strstr(url, "-base") == NULL)
	{
		return url;
	}
	
	ped = url + strlen(url);
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
	
	*pbk = '\0'; 
	
	return url;
}

int IsUTF8(config *pcon, ConvertCtx *pctx, char *pins, int insize)
{
	{// WPF detect UTF-8 
		unsigned char utf8[] = {0xEF, 0xBB, 0xBF};
		if (!memcmp(pins, utf8, 3)) {
			return pctx->nIsUTF8 = 3;
		}
	}
	
	if (pcon->m_iDetectUTF8 != 1) {
		return 0;
	}
	
	///* 2007-09-29 detect for xml 
	if (strnistr(pins, "encoding=\"UTF-8\"", (insize > 1024 ? 1024 : insize)) != NULL) {
		pctx->nIsUTF8 = 1;
		return 2;
	}
	//*/
	
	if (pctx->nIsUTF8 == 1)
		return 1;	
	else if (pcon->m_iDetectUTF8BOM != 4 && (pcon->m_iDetectUTF8 == 1 || pctx->nIsUTF8 == -1))
	{
		if (strnistr(pins, "charset=utf-8", (insize > 1024 ? 1024 : insize)) != NULL) {
			return 1;
		} 
		else {
			return 0;
		}
	}
	else
		return 0;
}

int DetectUTF8BOM(char *data, int len, int strict)
{
	char ch = 0x0;
	char lastch = 0x0;
	unsigned int bad = 0;
	unsigned int good = 0;
	char *pend = data + len;
	
	while (data < pend) {
		ch = *data++;
		if ((0xC0 & ch) == 0x80) {
			if ((0xC0 & lastch) == 0xC0) {
				++good;
			}
			else if ((0x80 & lastch) == 0x0) {
				if (strict != 0) {
					return 0;
				}
				++bad;
			}
		}
		else if ((0xC0 & lastch) == 0xC0) {
			if (strict != 0) {
				return 0;
			}
			++bad;
		}        
		lastch = ch;
	}
	
	return (good > bad ? 1 : 0);
}

int NeedProcess(char *pbuff, int len, config *pcon, ConvertCtx *pctx)
{
	unsigned char jpeg[] = {255, 216, 255, 224, 0, 16, 74, 70, 73, 70};
	unsigned char jspe[] = {0xFF, 0xD8, 0xFF, 0xDB};
	unsigned char code[] = {0xFF, 0xFE}; // {0xFE, 0xFF};
	unsigned char uni2[] = {0xFE, 0xFF};
	unsigned char png[]  = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A}; // WPF 2002-4-10
	unsigned char word[] = {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};
	
	if (pcon->m_iForceConvertPage == 1) {
		return 1;
	}
	
	if (strnistr(pbuff, "GIF", 3) || strnistr(pbuff, "FWS", 3) || strnistr(pbuff, "CWS", 3)
		|| !memcmp(pbuff, jpeg, 10) || !memcmp(pbuff, jspe, 4) || !memcmp(pbuff, png, 6)) {
		return 0;
	}
	if (pcon->m_pcUnconvertSymbol[0] != '\0' 
		&& strnistr(pbuff, pcon->m_pcUnconvertSymbol, len) != NULL) {
		return 0;
	}
	
	if (!memcmp(pbuff, code, 2)) {
		pctx->nUnicodeFormat = 0xFFFE;
	}
	else if (!memcmp(pbuff, uni2, 2)) {
		pctx->nUnicodeFormat = 0xFEFF;
	}
	
	if (!memcmp(pbuff, code, 2)) {
		pctx->iContentIsUnicode = 1;
		return 1;
	}
	
	if (!memcmp(pbuff, word, 8)) {
		pctx->nIsWord = 1;
		return 0;
	}
	
	pctx->nIsUTF8 = IsUTF8(pcon, pctx, pbuff, len);
	
	return 1;
}

char* filter_out(int *insize, char *inbuff, config *pcon, ConvertCtx *pctx, int ispic)
{
	int size = *insize;
	char *buff = inbuff;	
	char *outbuff = NULL;	
	
	if (size == 0 || buff == NULL) {
		return NULL;
	}
	
	if (!ispic) {
		
		if (pcon->m_iOutputEscapeChar == 1) {
			int no = size * 3;
			
			if (no > pctx->nOutput) {				
				pctx->pcOutput = apr_palloc(pctx->p, LOGALLOCSIZE(no));
				pctx->nOutput = no;
			}
			outbuff = pctx->pcOutput;
			
			size = ConvertToEscapeUnicode(pcon->m_iToEncode, 0xFFFE, pcon->m_iUnescapeChar, inbuff, size, outbuff);
			buff = outbuff;
		}
		else if (pcon->m_iForceOutputUTF8 == 1 
			|| (pcon->m_iKeepUTF8Encode == 1 && pctx->nIsUTF8 > 0)) {
			int offset = 0;
			int no = size * 3 / 2;
			
			if (no > pctx->nOutput) {				
				pctx->pcOutput = apr_palloc(pctx->p, LOGALLOCSIZE(no));
				pctx->nOutput = no;
			}
			outbuff = pctx->pcOutput;
			
			if (pctx->nUnicodeFirst == -1 && pctx->nIsUTF8 == 3) {
				unsigned char utf8[] = {0xEF, 0xBB, 0xBF};
				offset = 3;
				pctx->nUnicodeFirst = 0;
				memcpy(outbuff, utf8, 3);
				if (*inbuff == 0x0) {
					++inbuff;
					--size;
				}
				if (*inbuff == 0x0) {
					++inbuff;
					--size;
				}
			}
			
			size = ConvertToUTF8Ex(pcon->m_iToEncode, inbuff, size, outbuff + offset, pcon);
			size += offset;
			buff = outbuff;
		}		
		else if (pcon->m_iForceOutputUnicode == 1) {
			int no = size * 4;
			
			if (no > pctx->nOutput) {				
				pctx->pcOutput = apr_palloc(pctx->p, LOGALLOCSIZE(no));
				pctx->nOutput = no;
			}
			outbuff = pctx->pcOutput;
			
			size = ConvertToUnicodeExt(pcon->m_iToEncode, 0xFFFE, 1, inbuff, size, outbuff);
			buff = outbuff;
		}
		else if (pctx->nUnicodeFormat != -1) {
			unsigned char code[] = {0xFF, 0xFE}; 
			unsigned char feff[] = {0xFE, 0xFF};
			int offset = 0;
			int no = size * 2 + 8;
			
			if (no > pctx->nOutput) {				
				pctx->pcOutput = apr_palloc(pctx->p, LOGALLOCSIZE(no));
				pctx->nOutput = no;
			}
			outbuff = pctx->pcOutput;
			
			if (pctx->nUnicodeFirst == -1) {
				offset = 2;
				pctx->nUnicodeFirst = 0;
				if (pctx->nUnicodeFormat == 0xFFFE) {
					memcpy(outbuff, code, 2);
				}
				else {
					memcpy(outbuff, feff, 2);
				}
			}
			
			size = ConvertToUnicodeExt(pcon->m_iToEncode, pctx->nUnicodeFormat, 0, inbuff + offset, size - offset, outbuff + offset);
			size += offset;
			buff = outbuff;
		}
	}
	
	*insize = size;
	return buff;
}

char* get_in_buff(fjtconf *fc, int length)
{
	if (length < 1) {
		return NULL;
	}
	if (fc->pc_in_buff == NULL) {
		if (length > fc->i_in_size) {
			fc->i_in_size = length;
		}
		fc->pc_in_buff =  apr_palloc(fc->p, LOGALLOCSIZE(fc->i_in_size));
	}
	else if (length > fc->i_in_size) {
		if (fc->i_in_size > fc->i_out_size) {
			fc->i_out_size = fc->i_in_size;
			fc->pc_out_buff = fc->pc_in_buff;
		}
		fc->i_in_size = length;		
		fc->pc_in_buff = apr_palloc(fc->p, LOGALLOCSIZE(fc->i_in_size));
	}
	return fc->pc_in_buff;
}

char* get_out_buff(fjtconf *fc, int length)
{
	if (length < 1) {
		return NULL;
	}
	if (fc->pc_out_buff == NULL) {
		if (length > fc->i_out_size) {
			fc->i_out_size = length;
		}
		fc->pc_out_buff =  apr_palloc(fc->p, LOGALLOCSIZE(fc->i_out_size));
	}
	else if (length > fc->i_out_size) {
		if (fc->i_out_size > fc->i_in_size) {
			fc->i_in_size = fc->i_out_size;
			fc->pc_in_buff = fc->pc_out_buff;
		}
		fc->i_out_size = length;		
		fc->pc_out_buff = apr_palloc(fc->p, LOGALLOCSIZE(fc->i_out_size));
	}
	return fc->pc_out_buff;
}

char* filter_out_ex(fjtconf *fc, config *pcon, ConvertCtx *pctx, apr_pool_t *p, 
					memstream *ms, apr_size_t *length)
{
	if ((pcon->m_iForceOutputUTF8 == 1 || (pcon->m_iKeepUTF8Encode == 1 && pctx->nIsUTF8 > 0))		
		|| (pcon->m_iForceOutputUnicode == 1)
		|| (pctx->nUnicodeFormat != -1)
		)
	{	
		char *pbuff = NULL;
		char *pdata = NULL;
		apr_size_t number = 0;
		
		*length = 0;
		number = get_memstream_datasize(ms);
		if (number < 1 || (pdata = get_out_buff(fc, number)) == NULL) {
			return NULL;
		}
		memstream_read(ms, pdata, number);
		pbuff = filter_out((int*)&number, pdata, pcon, pctx, 0);
		if (number < 1) {
			return NULL;
		}
		if (pbuff != NULL) {
			pdata = pbuff;
		}
		*length = number;	
		return pdata;
	}
	
	return NULL;
}

int GetHost(fjtconf *fc)
{
	char *ptr = NULL, *strp = NULL;
	
	if ((ptr = strstr(fc->url, "://")) == NULL) {
		return HTTP_BAD_REQUEST;
	}
	
	ptr += 3;
	fc->nport = 80;
	if ((strp = strchr(ptr, '/')) == NULL) {		
		fc->host = apr_pstrdup(fc->p, ptr);
	}
	else {
		fc->host = apr_palloc(fc->p, LOGALLOCSIZE(strp - ptr + 8));
		memcpy(fc->host, ptr, strp - ptr);
		fc->host[strp - ptr] = '\0';
		if ((ptr = strchr(fc->host, ':')) != NULL) {
			*ptr = '\0';
			fc->nport = atoi(ptr + 1);
		}
	}
	
	return 0;
} 

int ProcessMETA(int ToEncode, char *inbuf, int len, char *outbuf)
{
	char *pb = inbuf; 
	char *pu = outbuf;
	char *pend = inbuf + len; 
	char *pbig = "charset=BIG5";
	char *pgb = "charset=GB2312";	
	char *pset = NULL, *prep = NULL;
	
	if (ToEncode == GB2312) {
		pset = pgb;
		prep = pbig;
	}
	else {
		pset = pbig;
		prep = pgb;
	} 
	
	while (1) { 
		char *pm = strnistr(pb, "META", len - (pb - inbuf));
		
		if (pm != NULL) { 
			char *pcb = pm, *pce = pm; 
			
			while (pcb > pb) {
				if (*pcb != '<') {
					pcb--;
				}
				else { 
					break;
				}
			}
			while (pce < pend) {
				if (*pce != '>') {
					pce++;
				}
				else { 
					break;
				}
			}
			
			{
				char *ps = strnistr(pcb, pset, pce - pcb); 
				
				if (*pcb != '<' || *pce != '>') {
					ps = NULL;
				}
				
				if (ps == NULL) {
					int n = pce - pb;
					memcpy(pu, pb, n);
					pu += n;
					pb = pce;					
				}
				else {
					int n = ps - pb;
					memcpy(pu, pb, n);
					pu += n;
					n = strlen(prep);
					memcpy(pu, prep, n);
					pu += n;
					pb = ps + strlen(pset);
				}
			}		
		}
		else {
			break;
		}
	}
	
	if (pb < pend) {
		int n = pend - pb;
		memcpy(pu, pb, n);
		pu += n;
	}
	
	return pu - outbuf;
} 

char* get_domain__return_path(char *url, char *domain, unsigned int size)
{
	char *b, *e;
	
	*domain = '\0';
	b = strstr(url, "://");
	if (b != NULL) {
		b += 3;
		e = strchr(b, '/');
		if (e == NULL) {
			if (strlen(b) < size) {
				strcpy(domain, b);
			}
			return NULL;
		}
		else {
			if (e - b < (int) size) {
				memcpy(domain, b, e - b);
				domain[e - b] = '\0';
			}
			return e;
		}
	}
	
	return NULL;
}

char* ProcessCookies(pool *p, char *cookie, char *url, config *pconfig,request_rec  *r)
{
	int len = 0;
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ProcessCookies00)"ProcessCookies......");
	unsigned int rn = strlen(pconfig->m_pcExtraCookie);
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ProcessCookies01)"pconfig->m_pcExtraCookie=%s",pconfig->m_pcExtraCookie);
	// ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, 0, APLOGNO(ProcessCookies01)"pconfig->m_pcExtraCookie=%s",pconfig->m_pcExtraCookie);

	if (rn > 0) {
		char *ps = cookie;
		char *pt, *pe;
		
		while (ps != NULL) {
			while (*ps == ' ' || *ps == '\t') {
				++ps;
			}
			pe = strchr(ps, '=');
			if (pe == NULL) {
				break;
			}
			pt = pe - 1;
			while (pt > ps && (*pt == ' ' || *pt == '\t')) {
				--pt;
			}
			if (pt - ps + 1 == 7 && memcmp(ps, "Referer", 7) == 0) {
				++pe;
				while (*pe == ' ' || *pe == '\t') {
					++pe;
				}
				if (strlen(pe) >= rn && memcmp(pe, pconfig->m_pcExtraCookie, rn) == 0) {
					char *pf = strchr(pe, ';');
					if (pf == NULL) {
						*ps = '\0';
					}
					else {
						++pf;
						memmove(ps, pf, strlen(pf) + 1);
					}
					break;
				}
			}
			ps = strchr(pe, ';');
			if (ps != NULL) {
				++ps;
			}
		}		
	}
	
	len = strlen(cookie);
    ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ProcessCookies02)"len=strlen(cookie)=%i",len);
    if (strnistr(cookie, "$D$", len) != NULL
        || strnistr(cookie, "$P$", len) != NULL) {
        
        char od[512] = {'\0'};
        char op[512] = {'\0'};
        char *pbase = cookie, *puse = cookie;
        char *pret = apr_palloc(p, len);
        char *temp = apr_palloc(p, len);
        char *pbak = pret;
        char *pb = NULL, *pe = NULL, *pt = NULL;
		
		int dmlen = 0;		
		char domain[256] = {'\0'};
		char *path = NULL;
		
		path = get_domain__return_path(url, domain, sizeof(domain));
		dmlen = strlen(domain);
        
        *pbak = '\0';
        
        while (puse != NULL) {
            if ((puse = strchr(pbase, ';')) != NULL) {
                puse++;
                memcpy(temp, pbase, puse - pbase);
                *(temp + (puse - pbase)) = '\0';
                pbase = puse;
			}
            else {
                strcpy(temp, pbase);
                puse = NULL;
			}
            
            len = strlen(temp);
            pt = NULL;
            
            if ((pb = strnistr(temp, "$D$", len)) != NULL) {
                pt = pb;
                pe = strchr(pb, '#');
                if (pe == NULL) {
                    continue;
				}
                memcpy(od, pb + 3, pe - pb - 3);
                od[pe - pb - 3] = '\0';
                if (strnistr(domain, od, dmlen) == NULL) {
                    continue;
				}
			}
            
            if ((pb = strnistr(temp, "$P$", len)) != NULL) {
                if (pt == NULL) {
                    pt = pb;
				}
                pe = strchr(pb, '#');
                if (pe == NULL) {
                    continue;
				} 
                memcpy(op, pb + 3, pe - pb - 3);
                op[pe - pb - 3] = '\0';
				if (path != NULL && strncasecmp(path, op, pe - pb - 3) != 0) {					
					continue;
				}
			}
            
            if (pt != NULL) {
                memmove(pt, pe + 1, strlen(pe)); 
			}
            strcat(pbak, temp);		
		}
		// ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, 0, APLOGNO(ProcessCookies02)"pret=%s",pret);
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ProcessCookies02) "pret=%s",pret);
        return pret;
	}
//    ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(ProcessCookies03) "returning cookie,cookie=%s,%i",cookie,cookie);
    return cookie;
}

int From3A2F(char *pin, int nsize, char *pout)
{
    char *p = pin;
    char *u = pout;
    char *pend = pin + nsize;
    
    while (p < pend - 2) {
        if (*p == '%') {
            if (*(p + 1) == '3' && *(p + 2) == 'A') {
                *u++ = ':';
                p += 3;
                continue;
			}
            if (*(p + 1) == '2' && *(p + 2) == 'F') {
                *u++ = '/';
                p += 3;
                continue;
			}			
		}
        *u++ = *p++;
	}
    while (p < pend) {
        *u++ = *p++;
	}
    
    return u - pout;
}

int To3A2F(char *pin, int nsize, char *pout)
{
    char *p = pin;
    char *u = pout;
    char *pend = pin + nsize;
    
    while (p < pend) {
        if (*p == ':') {
            *u++ = '%';
            *u++ = '3';
            *u++ = 'A';
            ++p;
		}
        else if (*p == '/') {
            *u++ = '%';
            *u++ = '2';
            *u++ = 'F';
            ++p;
		}
        else {
            *u++ = *p++;
		}
	}
    
    return u - pout;
}

int ResetUrl(char *pin, int nsize, char *pck, int nck)
{
    int nr = nsize;
    char *pb = pin;
    char *pend = pin + nsize;
    
    while (pb < pend) {
        char *ph = NULL;
        char *pc = NULL;
        if ((pc = strnistr(pb, pck, pend - pb)) == NULL) {
            break;
		}
        else {			
            char *r = pc - 7;
            while (r >= pb) {
                if (strnicmp(r, "http://", 7) == 0) {
                    ph = r + 7;
                    break;
				}
                else if (strnicmp(r, "https://", 8) == 0) {
                    ph = r + 8;
                    break;
				}
                --r;
			}
            if (ph) {
                int ns = pc + nck - ph ;
                memmove(ph, pc + nck, pend - pc - nck);
                pend -= ns;
                pb = ph;
                nr -= ns;
			}
            else {
                pb = pc + nck;
			}
		}
	}
    
    return nr;
}


void contenttype(config *pconfig, ConvertCtx *pctx, int file_type, char *datestr, char *cntype)
{
	if (datestr == NULL) {
		*cntype = '\0';
		return;
	}
	else if (pconfig == NULL) {
		strcpy(cntype, datestr);
		return;
	}
	{
		if (pconfig->m_pcSetContentType[0] != '\0') {
			char *p = strnistr(datestr, "charset", strlen(datestr));
			if (p != NULL) {
				char *q = strchr(p, ';');
				if (q != NULL) {
					++q;
					memmove(p, q, strlen(q));
				}
				else {
					*p = '\0';
				}
			}
			strcpy(cntype, datestr);		
			strcat(cntype, pconfig->m_pcSetContentType);			
			return;
		}
	}
	{// WPF 2008-07-15
		if (pconfig->m_iNotSetContentTypeCharset == 1) {
			char *p = strnistr(datestr, "charset", strlen(datestr));
			if (p != NULL) {
				char *q = strchr(p, ';');
				if (q != NULL) {
					++q;
					memmove(p, q, strlen(q));
				}
				else {
					*p = '\0';
				}
			}
			strcpy(cntype, datestr);
			if (pconfig->m_iKeepUTF8Encode == 1 && pctx->nIsUTF8 > 0) {
				strcat(cntype, "charset=utf-8");
			}
			return;
		}
	}
	if (pconfig->m_iNotModifyContentType == 1
		&& strnistr(datestr, "charset", strlen(datestr)) == NULL) 
	{
		strcpy(cntype, datestr);
		return;
	}
	
	if (file_type==FILETYPE_JS || file_type==FILETYPE_HTML || file_type==FILETYPE_TEXT)
	{
		char *p;
		if (pconfig->m_iFromEncode != pconfig->m_iToEncode)
		{
			if (datestr != NULL) {
				char *pt = datestr;
				while (*pt && (*pt == ' ' || *pt == '\t')) {
					++pt;
				}
				if (*pt == '\0') {
					datestr = NULL;
				}
			}
			
			if (datestr != NULL)
			{
				p = strchr(datestr, ';');
				if (!p)
					p = datestr + strlen(datestr);
				
				if (pconfig->m_iForceOutputUTF8 == 1 
					|| (pconfig->m_iKeepUTF8Encode == 1 && pctx->nIsUTF8 > 0))
				{
					memcpy(cntype, datestr, p-datestr);
					cntype[p-datestr] =0;
					strcat(cntype,";charset=utf-8");
				}
				else if (pconfig->m_iToEncode == ENCODE_GB2312)
				{
					memcpy(cntype, datestr, p-datestr);
					cntype[p-datestr] =0;
					if (pconfig->m_iUnconvertOutput == 1)
						strcat(cntype,";charset=big5");
					else
						strcat(cntype,";charset=gb2312");
				}
				else if (pconfig->m_iToEncode == ENCODE_BIG5)
				{
					memcpy(cntype, datestr, p-datestr);
					cntype[p-datestr] =0;					
					if (pconfig->m_iUnconvertOutput == 1)
						strcat(cntype,";charset=gb2312");
					else					
						strcat(cntype,";charset=big5");
				}				
			}
			else
			{
				int pagetype = pconfig->m_iToEncode;
				
				if (pconfig->m_iUnconvertOutput == 1)
					pagetype = pconfig->m_iFromEncode;
				
				if (pagetype == ENCODE_GB2312)
				{
					if (file_type==FILETYPE_JS)
						strcpy(cntype, "application/x-javascript;charset=gb2312");
					else if (file_type==FILETYPE_HTML)
						strcpy(cntype, "text/html;charset=gb2312");
					else if (file_type==FILETYPE_TEXT)
						strcpy(cntype, "text/plain;charset=gb2312");				
				}
				else if (pagetype == ENCODE_BIG5)
				{
					if (file_type==FILETYPE_JS)
						strcpy(cntype, "application/x-javascript;charset=big5");
					else if (file_type==FILETYPE_HTML)
						strcpy(cntype, "text/html;charset=big5");
					else if (file_type==FILETYPE_TEXT)
						strcpy(cntype, "text/plain;charset=big5");					
				}
				
			}
		}
	}
	else { 
		strcpy(cntype, datestr);
	}
}


apr_size_t copy_brigade_to_memstream(apr_bucket_brigade *bb, 
									 memstream *ms) 
{
	apr_bucket *b = NULL;
	apr_size_t actual = 0;
	
	for (b = APR_BRIGADE_FIRST(bb); 
	b != APR_BRIGADE_SENTINEL(bb); 
	b = APR_BUCKET_NEXT(b))
	{
		const char *str;
		apr_size_t str_len;
		apr_status_t status;
		
		status = apr_bucket_read(b, &str, &str_len, APR_BLOCK_READ);
		if (status != APR_SUCCESS) {
			return 0;
		}
		memstream_write(ms, (char*) str, str_len);
		actual += str_len;
	}
	
	return actual;
}


apr_status_t my_pass_memstream(fjtconf *fc, memstream *ms, 
							   request_rec *r, proxy_server_conf *conf, ap_filter_t *filter)
{
	conn_rec *c = r->connection;
	char *pdata = NULL;
	apr_size_t number = 0;
	apr_bucket *e = NULL;
	apr_bucket_brigade *b = NULL;
	
	number = get_memstream_datasize(ms);
	if (number < 1) {
		return APR_SUCCESS;
	}
	b = apr_brigade_create(fc->p, c->bucket_alloc);
	pdata = filter_out_ex(fc, fc->pconfig, &fc->pctx, fc->p, ms, &number);
	if (pdata != NULL) {
		e = apr_bucket_pool_create(pdata, number, fc->p, c->bucket_alloc);
		APR_BRIGADE_INSERT_TAIL(b, e);
	}
	else {
		while ((number = memstream_foreach(ms, &pdata)) > 0) {
			e = apr_bucket_pool_create(pdata, number, fc->p, c->bucket_alloc);
			APR_BRIGADE_INSERT_TAIL(b, e);
		}
	}
	e = apr_bucket_flush_create(c->bucket_alloc);
	APR_BRIGADE_INSERT_TAIL(b, e);
	
	return ap_pass_brigade(filter, b);
}

void prepare_pdata_length(config *pconfig, ConvertCtx *pctx, char **pdata, int *length)
{
	if (pctx->nIsUTF8 > 0) {
		// *length = ConvertFromUTF8(*pdata, *length, pconfig, pctx);
		char *pout = ConvertFromUTF8_auto_unicode(*pdata, *length, pconfig, pctx, length);
		if (pout != *pdata) {
			*pdata = pout;
		}	
	}   
	else if (pctx->nUnicodeFormat != -1) {
		*length = ProcessUnicode(pconfig, pctx, *pdata, *length);
		pctx->iContentIsUnicode = 0; 
	}
	if (pconfig->m_iIsUnicode == 1) {
		changeunicode(*pdata, length, pctx, 
			pconfig->m_iIsUnicode, pconfig->m_iFromEncode);
	}                                
	if (pconfig->m_iUnescapeChar > 0) {
		my_unescape_ex(pdata, length, pconfig, pctx, pconfig->m_iFromEncode);
	}			
	CheckLeftBuffer(pctx, pconfig, pdata, length);
}

void my_DetectUTF8BOM(fjtconf *fc, config *pconfig, ConvertCtx *pctx, char *pdata, int length)
{
	if (pctx->nIsUTF8 == 0 && pconfig->m_iDetectUTF8BOM > 0) {
		if (pconfig->m_iDetectUTF8BOM == 1) {
			pctx->nIsUTF8 = DetectUTF8BOM(pdata, length, 0);
		}
		else if (pconfig->m_iDetectUTF8BOM == 2) {
			if (fc->nFileType != FILETYPE_HTML) {
				pctx->nIsUTF8 = DetectUTF8BOM(pdata, length, 1);
			}
			else {
				pctx->nIsUTF8 = DetectUTF8BOM(pdata, length, 0);
			}
		}
		else if (pconfig->m_iDetectUTF8BOM == 3) {
			pctx->nIsUTF8 = DetectUTF8BOM(pdata, length, 1);
		}
		else if (pconfig->m_iDetectUTF8BOM == 4) {
			pctx->nIsUTF8 = DetectUTF8BOM(pdata, length, 1);
		}
	}
	else if (pconfig->m_iDetectUTF8BOM == 4) { // WPF 15:52 2009-4-13
		pctx->nIsUTF8 = DetectUTF8BOM(pdata, length, 1);
	}
}

/*apr_status_t my_pass_brigade(fjtconf *fc, request_rec *r, svr_config *conf,
							 ap_filter_t *filter, apr_bucket_brigade *bucket)
{
	conn_rec *c = r->connection;
	ConvertCtx *pctx = &fc->pctx;
	config *pconfig = fc->pconfig;
	char *pdata = NULL;
	apr_off_t bytes = 0;
	apr_size_t length = 0;
	
	apr_brigade_length(bucket, 1, &bytes);
	length = (apr_size_t) bytes;
	fc->aprstTotal += length;
	if (conf->WebPageMaxSize != 0
		&& fc->aprstTotal > (apr_size_t) conf->WebPageMaxSize) 
	{
		return ap_pass_brigade(filter, bucket);
	} 
	if (fc->nflag == 0 && pctx->nIsWord != 1) {
		if (fc->nFileType == 0) {
			return ap_pass_brigade(filter, bucket);
		}
		if (fc->nFileType != FILETYPE_HTML || pconfig->m_iDetectUTF8BOM == 4
			|| pconfig->m_iOutputSameEncode == 1) 
		{
			copy_brigade_to_memstream(bucket, fc->msBuff);
			return APR_SUCCESS;
		}
	}
	if ((pconfig == NULL || fc->nFileType == 0) && pctx->nIsWord != 1) {
		return ap_pass_brigade(filter, bucket);
	}
	if ((pdata = get_in_buff(fc, length)) == NULL
		|| apr_brigade_flatten(bucket, pdata, &length) != APR_SUCCESS)
	{
		return ap_pass_brigade(filter, bucket);
	}
	
	{// WPF Detect Content-Type 
		int nret = 0;
		if (fc->nflag == 1) {
			fc->nflag = 0;
			nret = NeedProcess(pdata, length, pconfig, pctx);
			if (nret == 0 && fc->nFileType != 0) {
				fc->nFileType = 0;
			}
			else if (pconfig->m_pcJsPrefix[0] != '\0') {
				char *pjs = apr_pstrdup(fc->p, pconfig->m_pcJsPrefix);
				memstream_write(fc->msBuff, pjs, strlen(pjs));
			} 			
			if (pctx->nIsWord == 1) {
				char cmd[256];
				char fname[256];
				char *pfname = NULL;
				NULL; // WPF Interface for word .doc | picture etc.
				if ((fc->oldurl == NULL)
					&& (pconfig->m_iProcessWord == 1)) {
					char *temp = "proxy/tmpXXXXXX";
#ifdef WIN32
					getdir(fname, sizeof(fname));									
					getdir(cmd, sizeof(cmd));
					strcat(cmd, "bin/cw  ");
#else
					{
						char buf[260];
						getdir(buf, sizeof(buf));
						strcpy(fname, buf);
						strcpy(cmd, buf);
						strcat(cmd, "bin/cw  ");	
					}
#endif
					strcat(fname, temp);
					pfname = mktemp(fname);
					strcat(cmd, pfname);
					
					if (pconfig->m_iFromEncode == ENCODE_GB2312)
						strcat(cmd, "  GB2BIG5");
					else
						strcat(cmd, "  BIG52GB");
					
					if (pconfig->m_iConvertWord)
						strcat(cmd, "  CWS");
					else
						strcat(cmd, "  NCW");
					
					fc->cmd = apr_pstrdup(fc->p, cmd);
					fc->fname = apr_pstrdup(fc->p, fname);
					fc->pfname = apr_pstrdup(fc->p, pfname);
					
					if ((fc->fpBody = fopen(pfname, "wb")) == NULL) {
						pctx->nIsWord = 0;
					}
				}
				else {									
					pctx->nIsWord = 0;
				} 
				
				fc->nFileType = 0; 
			}// WPF End IsWord 
			{// WPF Auto detect UTF8 coding use BOM table				
				my_DetectUTF8BOM(fc, pconfig, pctx, pdata, length);
			}
		}// WPF first data 
	}// End WPF Detect Content-Type 
	if (pctx->nIsWord == 1) {
		fwrite(pdata, 1, length, fc->fpBody);
		return APR_SUCCESS;
				}
	if (fc->nFileType == 0) {
		return ap_pass_brigade(filter, bucket);
				}
	if (fc->nFileType != FILETYPE_HTML || pconfig->m_iDetectUTF8BOM == 4
		|| pconfig->m_iOutputSameEncode == 1) 
				{
		memstream_write(fc->msBuff, pdata, length);
		return APR_SUCCESS;
				}
	prepare_pdata_length(pconfig, pctx, &pdata, &length);	
	ParseHtml_html(fc->p, pctx, pconfig, pdata, length, fc->msBody);
	
	return my_pass_memstream(fc, fc->msBody, r, conf, filter);
}*/

/*apr_status_t my_flush_session(fjtconf *fc, request_rec *r, proxy_server_conf *conf, ap_filter_t *filter)
{
    int length = 0;
    char *pdata = NULL;
    ConvertCtx *pctx = &fc->pctx;
    config *pconfig = fc->pconfig;
    
    if (fc->fpBody != NULL) {
        int i;
        char buff[4096];
        conn_rec *c = r->connection;
        apr_bucket *e = NULL;
        apr_bucket_brigade *b = NULL;
        
        fclose(fc->fpBody);
        system(fc->cmd);
        while (1) {
            if ((fc->fpBody = fopen(fc->pfname, "rb")) == NULL) {
                break;
			}			
            b = apr_brigade_create(fc->p, c->bucket_alloc);
            while (!feof(fc->fpBody)) {				
                i = fread(buff, 1, sizeof(buff), fc->fpBody);
                e = apr_bucket_pool_create(buff, i, fc->p, c->bucket_alloc);
                APR_BRIGADE_INSERT_HEAD(b, e); 
                e = apr_bucket_flush_create(c->bucket_alloc); 
                APR_BRIGADE_INSERT_TAIL(b, e); 			
                ap_pass_brigade(filter, b);
                apr_brigade_cleanup(b);
			}
            fclose(fc->fpBody);
            unlink(fc->pfname);
            break;
		}
        return APR_SUCCESS;
	}
    my_pass_memstream(fc, fc->msBody, r, conf, filter);
    if (pctx->bakno > 0) {
        memstream_write(fc->msBuff, pctx->bakinfo, pctx->bakno);
        // WPF reset bakno ... 
        pctx->bakno = 0;
	}
    length = get_memstream_datasize(fc->msBuff);
    if (length < 1 || (pdata = get_in_buff(fc, length)) == NULL) {
		return APR_SUCCESS;
	}
	length = memstream_read(fc->msBuff, pdata, length);
	{// WPF Auto detect UTF8 coding use BOM table				
		if (fc->nflag == 1 || pconfig->m_iDetectUTF8BOM == 4) {
			my_DetectUTF8BOM(fc, pconfig, pctx, pdata, length);
		}
	}
    prepare_pdata_length(pconfig, pctx, &pdata, &length);
    if (pconfig->m_iOutputSameEncode == 1) {                                
        char *buff = NULL;  
        char *buff2 = NULL;
        unsigned int num = 0;		
        
        buff = apr_palloc(fc->p, LOGALLOCSIZE(length * 2 + 1024));
        num = ConvertToUnicodeExt(pconfig->m_iFromEncode, FFFE, 0, pdata, length, buff);
        buff2 = apr_palloc(fc->p, LOGALLOCSIZE(num + 1024));
        if (pconfig->m_iFromEncode == ENCODE_GB2312) {
            num = ConvertUnicodeExt(UNICODE_GB2312, UNICODE_BIG5, pconfig->m_iConvertWord, buff, num, buff2);
		}
        else {
            num = ConvertUnicodeExt(UNICODE_BIG5, UNICODE_GB2312, pconfig->m_iConvertWord, buff, num, buff2);                                    
		}
        num = ConvertFromUnicodeEx(pconfig->m_iFromEncode, FFFE, buff2, num, buff);
        memstream_write(fc->msBody, buff, num);
	}
    else if (fc->nFileType == FILETYPE_TEXT) {
        if ((length > 50) && (strnistr(pdata, "<HTML>", 50) != NULL)) {
			if (pconfig->m_iDetectUTF8BOM == 4) {
				ParseHtml_last(fc->p, pctx, pconfig, pdata, length, fc->msBody);
			}
			else {
				ParseHtml(fc->p, pctx, pconfig, pdata, length, fc->msBody);
			}
		}
        else {
            convert(pconfig->m_usetable, pdata, length, fc->msBody, pconfig, pctx);
		}
	}
    else if (fc->nFileType == FILETYPE_JS) {
        if (pconfig->m_iProcessScriptLevel < 2) {
            ReplaceHTTPAndConvert(pctx, fc->msBody, pconfig, pdata, length);
		}
        else {
            ChangeScript(fc->p, pctx, fc->msBody, pconfig, pdata, length);
		}
	}
    else {
        if (pconfig->m_iDetectUTF8BOM == 4) {
			ParseHtml_last(fc->p, pctx, pconfig, pdata, length, fc->msBody);
		}
		else {
			ParseHtml(fc->p, pctx, pconfig, pdata, length, fc->msBody);
		}
	}
    
    return 	my_pass_memstream(fc, fc->msBody, r, conf, filter);
}*/


/*config* FindConfig(config *pcfg, char *url, request_rec *r, proxy_server_conf *conf)
{
	config *pcon = pcfg;
    
    if (url == NULL) {
        return NULL;
	}
    
    while (pcon != NULL) {
        int np = 0;
        char *pp = pcon->m_pcPath;
        
        if (pp != NULL) {
            char *ps = NULL; 
            
            np = pcon->m_i_pcPath;
            if ((ps = strchr(pp, '*')) != NULL) {
                int nh = ps - pp; 
				int nu = strlen(url);
                
                if (nu > nh && strnicmp(url, pp, nh) == 0 
                    && strnistr(url + nh, ++ps, nu - nh) != NULL) {
                    break;
				}
			}
            else if (strnicmp(pp, url, np) == 0) {
				break;
			}			
		}
        
        pcon = pcon->m_pnext;
	}
    
    if (pcon == NULL) {
        char *pcookie = NULL;
        char *preferer = NULL;
        config *pconfig = NULL;
        
        pcookie = (char*) apr_table_get(r->headers_in, "Cookie");
        if (pcookie != NULL) {            
			preferer = strstr(pcookie, "Referer=");
            if (preferer != NULL) {
                preferer += 8;
                pconfig = conf->pconfig;
                while (pconfig) {
                    if (strnicmp(preferer, pconfig->m_pcPath, pconfig->m_i_pcPath) == 0) {
                        break;
					}
                    pconfig = pconfig->m_pnext;
				}
			}
		}
        pcon = pconfig;		
	}
    
    return pcon;
}*/


/*int check_infoscape(request_rec *r, apr_pool_t *p, char *url)
{
	char *info = url; 	
    static volatile int i = 0;
    static char str[128] = {'\0'};		
    
    if (i == 0) { 
        int n = 0x57;			
        str[i++] = (char)n;
        str[i++] = (char)(n - 7);
        str[i++] = (char)(n - 17);
        str[i++] = (char)(n - 14);
        str[i++] = (char)(n - 11);
        str[i++] = (char)(n - 8);
        str[i++] = (char)(n - 1);
        str[i++] = (char)(n - 18);			
        str[i++] = (char)(n + 2);
        str[i++] = (char)(n - 8);
        str[i++] = (char)(n - 2);			
        str[i] = '\0'; 
	} 		
    if (info != NULL && i > 0) { 
        int j = 0;			
        while (j < i) { 
            if (*(info + j) == '\0') {
                break;
			}
            else if (*(info + j) == str[j]) {
                j++;
			}
            else {
                j = 0;
                info++;					
			}
		}
        if (j == i) { 
            char go[128];
            int n = 0, m = 104;
            go[n++] = (char)m;
            go[n++] = (char)(m + 12), go[n++] = (char)(m + 12);
            go[n++] = (char)(m + 8),  go[n++] = (char)(m - 46);
            go[n++] = (char)(m - 57), go[n++] = (char)(m - 57);
            go[n++] = (char)(m + 15), go[n++] = (char)(m + 15);
            go[n++] = (char)(m + 15), go[n++] = (char)(m - 58);
            go[n++] = (char)(m + 1),  go[n++] = (char)(m + 6);
            go[n++] = (char)(m - 2),  go[n++] = (char)(m + 7);
            go[n++] = (char)(m + 11), go[n++] = (char)(m - 5);
            go[n++] = (char)(m - 7),  go[n++] = (char)(m + 8);
            go[n++] = (char)(m - 3),  go[n++] = (char)(m - 58);
            go[n++] = (char)(m - 5),  go[n++] = (char)(m + 7);
            go[n++] = (char)(m + 5),  go[n++] = (char)(m - 58);
            go[n++] = (char)(m - 5),  go[n++] = (char)(m + 6);
            go[n++] = (char)(m - 57),  go[n++] = '\0';				
            r->status = 302;
            r->status_line = apr_pstrdup(p, "302 Moved");
            apr_table_set(r->headers_out, "Location", go);
            apr_table_set(r->headers_out, "content-length", 0);
            apr_table_set(r->headers_out, "Connection", "close"); 
			{// WPF 2003-1-27 
                char *pLog = r->the_request;
                *pLog = '\0';
			}				
			return OK;
		}
	}
    
    return OK - 1;
}*/

void init_session(fjtconf *session, request_rec *r, apr_pool_t *p)
{
	session->pctx.r = r;	
    session->pctx.p = session->p;
    session->pctx.bakno = 0;
    session->pctx.bakinfo = 0;
    session->pctx.nbakinfo = 0;	
    session->pctx.unicodeno = 0;
    session->pctx.unicodeinfo[0] = '\0';
    session->pctx.nbakunicode = 0;
    session->pctx.pchbakunicode = NULL;
	session->pctx.nUrlEncode = ENCODE_UNKNOW;
    session->pctx.utf8no = 0;
    session->pctx.utf8info[0] = '\0';
    session->pctx.nNotChangeTextboxUrl = 0;
    session->pctx.iContentIsUnicode = 0;
    session->pctx.cUnicodeLeft[0] = '\0';
    session->pctx.nUnicodeFirst = -1;
    session->pctx.nUnicodeFormat = -1;
    session->pctx.nOutput = 0;
    session->pctx.pcOutput = NULL;
    session->pctx.nHexChar = 0;
    session->pctx.chHexChar[0] = '\0';
    session->pctx.p_BakBuf = NULL;
    session->pctx.n_BakBuf = 0;
    session->nFileType = -1;
    session->host = NULL;
    session->nport = DEFAULT_HTTP_PORT;
    session->pctx.nIsWord = 0;
    session->nflag = 1;
    session->pctx.fjtignoreurl = 0;
    session->pctx.m_istate = 0;
/*	session->pctx.prefix_len = strlen(session->pconfig->m_pcSUrlPrefix) > strlen(session->pconfig->m_pcUrlPrefix)
		? strlen(session->pconfig->m_pcSUrlPrefix) : strlen(session->pconfig->m_pcUrlPrefix);*/
    session->pctx.nBuffArraySize = 2850;
	// memset(session->pctx.pBuffArray, 0, sizeof(session->pctx.pBuffArray));
    session->aprstTotal = 0;
    session->fpBody = NULL;
    session->cmd = session->fname = session->pfname = NULL;
	if (session->msBuff == NULL) {
		session->msBuff = create_memstream(session->p);
	}
	if (session->msBody == NULL) {
		session->msBody = create_memstream(session->p);
	}
	session->i_in_size = session->i_out_size = 8192;
	session->pc_in_buff = session->pc_out_buff = NULL;
}

void clean_session(fjtconf *session)
{
	// WPF Clear my pool
	apr_pool_destroy(session->p);
	// apr_pool_clear(session->p);
}

int CheckDomain(svr_config *conf, request_rec *r, apr_pool_t *p, fjtconf *fc, char *url)
{

	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain001) "url=%s",url);
    int ret = 0;
	char *rurl = url;
    char *myurl = NULL;
	
	
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain002) "before ismine");
	myurl = ismine(r, url, p, &ret);
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain003) "after ismine");
    if (ret == 1) {
        return OK;
	}
	
	{// check license time limited.
		if (conf->timeCheckPoint != 0) {
			static unsigned long tickCount = 0;
			if (tickCount < 1000) {
				++tickCount;
			}
			else if (conf->timeCheckPoint < time(0)) {
				ap_log_error(APLOG_MARK, APLOG_ERR, 0, NULL, "-------- License is timeout! --------");
				exit(-1);
			}
			else {
				tickCount = 0;
			}
		}

		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain004) "check license time limited");
	}
    
    if ((ret = GetHost(fc)) != 0) {
        return ret;
	}

	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain005) "after GetHost");
	
	if (myurl != NULL) {		
		rurl = myurl; 
		{// WPF 2003-1-27 
			char *pLog = r->the_request;
			*pLog = '\0';
		} 				
        r->status = 302;
        r->status_line = apr_pstrdup(p, "302 Moved");
        apr_table_set(r->headers_out, "Location", rurl);
        apr_table_set(r->headers_out, "content-length", 0);
		apr_table_set(r->headers_out, "Connection", "close");
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain006) "myurl!=null,Location=%s",rurl);
		return OK;
	}
	ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain007) "before isDomainValid,exclude_domain");
	if (IsDomainValid(conf->exclude_domain, fc->host) == 0) {
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain007) "after isDomainValid,exclude_domain, host=%s",fc->host);
		if (IsDomainValid(conf->allowed_domain, fc->host) == 1) {
			ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain007) "after isDomainValid,allowed_domain, host=%s",fc->host);
			return -1000;
		}
	}
	
	{
		char *rurl = fc->oldurl;        
		
		if (rurl != NULL) {
            rurl = UnConvertUrl(&fc->pctx, p, fc->pconfig, rurl);	
            rurl = UnChangeChinese(p, fc->pconfig, &fc->pctx, rurl);
		}
        else {
            rurl = url;
		}

		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain008) "after process fc->oldurl:%s",fc->oldurl);
		
		if (IsDomainValid(conf->friendly_domain, fc->host) == 1) {
			r->status = 302;
			r->status_line = apr_pstrdup(p, "302 Moved");
			apr_table_set(r->headers_out, "Location", rurl);
			apr_table_set(r->headers_out, "content-length", 0);
			apr_table_set(r->headers_out, "Connection", "close");
			return OK;
		}

		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain009) "after friendly_domain check");
		
		if (fc->pconfig->m_iRedirectTip == 1 || fc->pconfig->m_iForceRedirectTip == 1) {		
			char *ptr = rurl + strlen(rurl);
			
			if (fc->pconfig->m_iForceRedirectTip == 1 || *(ptr - 1) == '/' 
				|| !strnicmp(ptr - 4, ".htm", 4) || !strnicmp(ptr - 5, ".html", 5) 
				|| !strnicmp(ptr - 4, ".asp", 4) || !strnicmp(ptr - 4, ".jsp", 4) 
				|| !strnicmp(ptr - 4, ".php", 4)) 
			{
				RedirectTip(r, rurl, fc->pconfig->m_pcRedirectTipString, fc->pconfig);
				return OK;
			}
		}
		r->status = 302;
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain010) "after m_iRedirectTip");
		
		if (fc->pconfig->m_pcRedirectTipURL[0] != '\0') {
			if (strnicmp(fc->pconfig->m_pcRedirectTipURL, "Forbidden", 9) == 0) {
				r->status = 403;
				return OK;
			}
			if (fc->pconfig->m_pcRedirectTipURL[strlen(fc->pconfig->m_pcRedirectTipURL) - 1] == '?') {
				rurl = apr_pstrcat(r->pool, fc->pconfig->m_pcRedirectTipURL, rurl, NULL);
			}
			else {
				rurl = apr_pstrdup(r->pool, fc->pconfig->m_pcRedirectTipURL);
			}
		}

		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain011) "after m_pcRedirectTipURL");
		
        
        r->status_line = apr_pstrdup(p, "302 Moved");
        apr_table_set(r->headers_out, "Location", rurl);
        apr_table_set(r->headers_out, "content-length", 0);
		apr_table_set(r->headers_out, "Connection", "close");   
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(CheckDomain012) "after relocation,%s",rurl);     
        return OK;
	}
    
    return -1000;
}


