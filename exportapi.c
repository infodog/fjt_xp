#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#ifndef   WIN32	 
#include  <sys/types.h>
#include  <sys/mman.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <fcntl.h>
#endif

#include  "config.h"
#include  "exportapi.h"
#include  "memstream.h"

#define   LOG(x)

/*-------------------------------------------------------------------------------------*/

unsigned  char *pShareGB, *pShareBIG;
ruletable *pWordGB, *pWordBIG;
char *pU2GB, *pU2BIG, *pGB2U, *pBIG2U;
unsigned char *UniGbBig, *UniBigGb;
ruletable *UniGbBigRule, *UniBigGbRule;

/*-------------------------------------------------------------------------------------*/

int EncodeWChar(char *pin, int size, char *pout, int flag)
{
    int i, n, nlen;	
    unsigned char ch, *pl, *pr;
    unsigned char *pd, *pt, *pend;
    
    pd = (unsigned char*)pin;
    pt = (unsigned char*)pout;
    pend = pd + size;
    
    if (flag == 0) {
        while (pd < pend) {
            *pt++ = *pd / 16 + 'A';
            *pt++ = *pd++ % 16 + 'A';
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
            ch = (ch - 'A') * 16 + *pd++ - 'A';							
            *pt++ = ch;
        }
        nlen = (char*)pt - pout;	
    }
    
    return nlen;
}

/*-------------------------------------------------------------------------------------*/
/* Share word and words rule table (use for GB2312 or BIG5) */

extern void getdir(char *dir,int len);

int InitShareTable()
{
    char buf[128], dir[1024];
    FILE *fp;
    unsigned char c1,c2,c3,c4;
    int nlow = 2, high = 256*2, memsize = 256*256*2;
    
    buf[0] = '\0';
    
    memset(pShareGB, 255, memsize);
    memset(pShareBIG, 255, memsize);
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
        *(pShareGB+c3*high+c2*nlow)   = c1;
        *(pShareGB+c3*high+c2*nlow+1) = c4;		
#else
        *(pShareGB+c1*high+c2*nlow)   = c3;
        *(pShareGB+c1*high+c2*nlow+1) = c4;		
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
        *(pShareBIG+c3*high+c2*nlow)   = c1;
        *(pShareBIG+c3*high+c2*nlow+1) = c4;		
#else
        *(pShareBIG+c1*high+c2*nlow)   = c3;
        *(pShareBIG+c1*high+c2*nlow+1) = c4;		
#endif		
    }
    fclose(fp);
    
    return 1;
}

int InitShareFile(ruletable *wordlist, char *pBoth, char *filename)
{
    FILE* fp;
    int	  flag, word, lenreal, lenrep;
    char  *ptr, line[RULELEN*2+10], realcode[RULELEN], repcode[RULELEN];
    rule  *element;	
    
    int   nNode    = sizeof(ruletable);
    int   nSubNode = sizeof(rule);
    char  *pBase;
    char  *pForBase = (char*)wordlist;
    ruletable *pUseWord;
    pBase = pForBase + nNode*FJTMAXWORD;
    if (pBoth == NULL)
        pBoth = pBase;
    else
        pBase = pBoth;
    
    if ((fp=fopen(filename,"r"))==NULL) return(0);
    
    wordlist->flag = (int)wordlist;
    while (!feof(fp)) {
#ifdef ENCRYPT_WORDS			
        {/* WPF 2002-8-9 Decode line */
            int n;
            char temp[RULELEN * 4 + 10];
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
        if (!(ptr = strchr(line, '\t')) || ptr == line)
            continue;
        *ptr = '\0';		
        lenreal = strlen(line);
        if (!lenreal || (lenreal > RULELEN))
            continue;
        strcpy(realcode, line);
        lenrep = strlen(ptr + 1);
        if (!lenrep || (lenrep > RULELEN))
            continue;
        strcpy(repcode, ptr+1);
        
        word = realcode[0];
        if ((word & 0x80)!=0) {
            unsigned char chi = realcode[0];
            unsigned char chw = realcode[1];
            word = chi * 256 + chw;
        }
        if (word == 0)
            continue;
        
        element  = (rule*)pBase;
        pForBase = pBase;
        pForBase += nSubNode;
        pBase = pForBase;
        pBoth = pBase;
        element->lenreal=lenreal;
        element->lenrep=lenrep;
        strcpy(element->realcode,realcode);
        strcpy(element->repcode,repcode);
        element->flag=flag;
        
        {
            char *pTemp = (char*)wordlist;
            pTemp += nNode*word;
            pUseWord = (ruletable*)pTemp;
        }
        
        if (pUseWord->body == NULL) {
            element->link=NULL;	
            pUseWord->body=element;
            pUseWord->flag=flag;
        }
        else {
            element->link=pUseWord->body;
            pUseWord->body=element;
            if (flag && !(pUseWord->flag))
                pUseWord->flag=flag;
        }
    } 
    
    fclose(fp); 	
    return(1);
}

int InitWordTable(char *ptable,char *filename){
    FILE *fp;
    unsigned char buff[4];
    int low = 2, high = 256 * 2;
    if ((fp = fopen(filename, "rb")) == NULL) return 0;
    while (!feof(fp)) {
        if (fread(buff, 1, 4, fp) != 4) break;
        int offset = high * buff[0] + low * buff[1];
        ptable[offset] = buff[2];
        ptable[offset+1] = buff[3];
    }

    fclose(fp);
    return 1;
}
/*-------------------------------------------------------------------------------------*/
/* Share word and words rule table (use for UNICODE TO UNICODE) */

int InitUnicodeWord(char *pbuff, char *filename)
{
   return InitWordTable(pbuff,filename);
}

int getline2(char *line, int len, FILE *fp)
{
    char *ptr = line;
    unsigned char buff[2];
    
    while (!feof(fp))
    {
        if (fread(buff, 1, 2, fp) != 2)			
            break;
        else if (buff[0] == 0xA && buff[1] == 0x0)
        {
            ptr -= 2;
            break;
        }
        else
        {
            if (ptr + 2 > line + len)
                return 0;
            memcpy(ptr, buff, 2);
            ptr += 2;
        }
    }
    
    return (ptr - line) % 2 ? 0 : (ptr - line);
}

int InitUnicodeRule(ruletable *wordlist, char *psem, char *filename)
{
    
    FILE* fp;
    int	  flag, word, lenreal, lenrep;
    char  line[RULELEN*2+10], realcode[RULELEN], repcode[RULELEN];
    rule  *element;
    
    int   nNode    = sizeof(ruletable);
    int   nSubNode = sizeof(rule);
    char  *pbase;
    ruletable *pUseWord;
    
    if (psem) pbase = psem;
    else {
        pbase = (char *)wordlist + nNode * FJTMAXWORD;
        psem = pbase;
    }	
#ifdef ENCRYPT_WORDS		
    if ((fp = fopen(filename, "r")) == NULL) return(0);
#else
    if ((fp = fopen(filename, "rb")) == NULL) return(0);
#endif
    
    wordlist->flag = (int)wordlist;
    while (!feof(fp)) {
#ifdef ENCRYPT_WORDS		
    /*
    int len = getline2(line, sizeof(line), fp);
        */
        int len;
        {/* WPF 2002-8-9 Decode the line */
            char temp[RULELEN * 4 + 10];
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
            len = strlen(temp);
            len = EncodeWChar(temp, len, line, 1);
        }
#else
        int len = getline2(line, sizeof(line), fp);
#endif		
        if (!len) break;
        
        {/* Get rule from table! */
            char *ptr;
            lenreal = lenrep = 0;
            
            for(ptr = line; ptr < line + len; ) {
                if ((unsigned char)*ptr == 0x9 && (unsigned char)*(ptr + 1) == 0x0) {
                    lenreal = ptr - line;
                    if (lenreal > RULELEN) break;
                    memcpy(realcode, line, lenreal);
                    lenrep = len - lenreal - 2;
                    if (lenrep > RULELEN) {
                        lenrep = 0;
                        break;
                    }
                    memcpy(repcode, ptr + 2, lenrep);
                    break;
                }
                else
                    ptr += 2;
            }
        }
        
        if (!lenreal || !lenrep) continue;
        flag = 0;
        
        {
            word = (unsigned char)realcode[0] * 256 + (unsigned char)realcode[1];
            if (word == 0) continue;
            element  = (rule *)pbase;
            pbase += nSubNode;
            psem = pbase;
            element->lenreal = lenreal;
            element->lenrep = lenrep;
            memcpy(element->realcode, realcode, lenreal);
            memcpy(element->repcode, repcode, lenrep);
            element->flag = flag;
            
            pUseWord = (ruletable*)((char *)wordlist + nNode * word);			
            if (pUseWord->body == NULL) {
                element->link = NULL;	
                pUseWord->body = element;
                pUseWord->flag = flag;
            }
            else {
                element->link = pUseWord->body;
                pUseWord->body = element;
                if (flag && !(pUseWord->flag))
                    pUseWord->flag = flag;
            }
        }		
    }
    
    fclose(fp);
    return(1);
}

/*------------------------------------------------------------------------------*/
/* Use (12345 256256) unicode table */

int ForUnicode(char *pFrom, char *pTo, char *pFileName)
{
    FILE *fp;
    int  nNext = sizeof(char)*2;
    char left[10], right[10], line[1024];
    
    if ((fp = fopen(pFileName, "rb")) == NULL)
        return 0;
    
    while(!feof(fp))
    {
        int  nL, nR;
        char *p, *ptr;
        
        fgets(line, sizeof(line), fp);
        if (!(p = strchr(line, '\t')))
            continue;
        
        memcpy(left, line, p-line);
        left[p-line] = '\0';		
        strcpy(right, p+1);				
        nL = atoi(left);
        nR = atoi(right);
        nR = ((nR/1000) << 8) | (nR%1000);
        
        if (!nL || !nR)
            continue;
        
        ptr = pFrom + nNext*nL;
        *ptr = (char)(nR >> 8);
        *(ptr+1) = (char)nR;
        ptr = pTo + nNext*nR;
        *ptr = (char)(nL >> 8);
        *(ptr+1) = (char)nL;
    }
    
    fclose(fp);
    return 1;
}

/*-------------------------------------------------------------------------------------*/

int InitShareApi(void)
{
    int buffsize = 256*256*2;
    // int heapsize = 4096*400;
	int heapsize = sizeof(ruletable) * FJTMAXWORD + sizeof(rule) * 8192;
    
    pShareGB = pShareBIG = NULL;
    pWordGB = pWordBIG = NULL;
    pU2GB = pU2BIG = pGB2U = pBIG2U = NULL;
    UniGbBig = UniBigGb = NULL;
    UniGbBigRule = UniBigGbRule = NULL;
    
#ifdef WIN32
    {/* This code use for Share memory Under Win32 */
        HANDLE l_hgb, l_hbig5, l_hgbword, l_hbig5word;
        HANDLE u2gb, u2big, gb2u, big2u;
        HANDLE ugb, ubig, ugbrule, ubigrule;
        
        l_hgb    = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVargb_Info");		
        pShareGB = (unsigned char*)MapViewOfFile(l_hgb, FILE_MAP_WRITE, 0, 0, 0);	
        l_hbig5    = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVarbig5_Info");
        pShareBIG  = (unsigned char*)MapViewOfFile(l_hbig5, FILE_MAP_WRITE, 0, 0, 0);						
        l_hgbword  = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, heapsize, "ShareConvertVargbword_Info");
        pWordGB    = (ruletable*)MapViewOfFile(l_hgbword, FILE_MAP_WRITE, 0, 0, 0);						
        l_hbig5word  = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, heapsize, "ShareConvertVarbig5word_Info");
        pWordBIG     = (ruletable*)MapViewOfFile(l_hbig5word, FILE_MAP_WRITE, 0, 0, 0);
        
        u2gb = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVaru2gb_Info");		
        pU2GB = (char*)MapViewOfFile(u2gb, FILE_MAP_WRITE, 0, 0, 0);
        u2big = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVaru2big_Info");		
        pU2BIG = (char*)MapViewOfFile(u2big, FILE_MAP_WRITE, 0, 0, 0);
        gb2u = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVargb2u_Info");		
        pGB2U = (char*)MapViewOfFile(gb2u, FILE_MAP_WRITE, 0, 0, 0);
        big2u = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVarbig2u_Info");		
        pBIG2U = (char*)MapViewOfFile(big2u, FILE_MAP_WRITE, 0, 0, 0);
        
        ugb    = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVarunicodegb_Info");		
        UniGbBig = (unsigned char*)MapViewOfFile(ugb, FILE_MAP_WRITE, 0, 0, 0);	
        ubig    = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, buffsize, "ShareConvertVarunicodebig_Info");
        UniBigGb  = (unsigned char*)MapViewOfFile(ubig, FILE_MAP_WRITE, 0, 0, 0);						
        ugbrule  = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, heapsize, "ShareConvertVarunicodegbrule_Info");
        UniGbBigRule    = (ruletable*)MapViewOfFile(ugbrule, FILE_MAP_WRITE, 0, 0, 0);						
        ubigrule  = CreateFileMapping((HANDLE)(-1), 0, PAGE_READWRITE, 0, heapsize, "ShareConvertVarunicodebigrule_Info");
        UniBigGbRule     = (ruletable*)MapViewOfFile(ubigrule, FILE_MAP_WRITE, 0, 0, 0);
        
        if (!l_hgb || !l_hbig5 || !l_hgbword || !l_hbig5word || !u2gb || !u2big || !gb2u || !big2u || (!ugb || !ubig || !ugbrule || !ubigrule))			
            LOG("ERROR in initialization Mapping file!\n");
        else
        {
            char dir[260];
            
            {/* Share GB and BIG5 word and rule */
                if (!InitShareTable())
                    return 0;
                
                memset(pWordGB, 0, heapsize);
                memset(pWordBIG, 0, heapsize);
                
                getdir(dir,sizeof(dir));
                strcat(dir,"etc\\gb2big5.cvt2");	
                {
                    char *ptr = NULL;
                    if (!InitShareFile(pWordGB, ptr, dir))
                        return 0;
                }				
                getdir(dir,sizeof(dir));
                strcat(dir,"etc\\big52gb.cvt1");
                {
                    char *ptr = NULL;
                    if (!InitShareFile(pWordBIG, ptr, dir))
                        return 0;
                }
            }
            
            {/* Share Unicode word table */
                memset(UniGbBig, 0, buffsize);
                memset(UniBigGb, 0, buffsize);
                
                getdir(dir,sizeof(dir));
                strcat(dir,"etc\\ungb2bigword.txt");
                if (!InitUnicodeWord(UniGbBig, dir))
                    return 0;
                
                getdir(dir,sizeof(dir));
                strcat(dir,"etc\\unbig2gbword.txt");
                if (!InitUnicodeWord(UniBigGb, dir))
                    return 0;
            }
            
            {/* 引入互斥量机制初始化 Unicode To Unicode 词表 */
                HANDLE hMutex = NULL;
                
                if ((hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "InitConvertAPI")) == NULL)
                    hMutex = CreateMutex(NULL, FALSE, "InitConvertAPI");
                
                WaitForSingleObject(hMutex, INFINITE);
                
                {/* Share Unicode rule table */
                    memset(UniGbBigRule, 0, heapsize);
                    memset(UniBigGbRule, 0, heapsize);
                    
                    getdir(dir, sizeof(dir));
                    strcat(dir, "etc\\ungb2bigrule.txt");		
                    {
                        char *ptr = NULL;
                        if (!InitUnicodeRule(UniGbBigRule, ptr, dir))				
                            return 0;
                    }
                    getdir(dir, sizeof(dir));
                    strcat(dir, "etc\\unbig2gbrule.txt");		
                    {
                        char *ptr = NULL;
                        if (!InitUnicodeRule(UniBigGbRule, ptr, dir))
                            return 0;
                    } 
                }
                
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
            }
            
            {/* Share Unicode table format is &#12345; */
                memset(pU2GB, 255, buffsize);
                memset(pU2BIG, 255, buffsize);
                memset(pGB2U, 255, buffsize);
                memset(pBIG2U, 255, buffsize);
                
                getdir(dir, sizeof(dir));
                strcat(dir, "etc\\unicodetogb.txt");
                if (!ForUnicode(pU2GB, pGB2U, dir))
                    return 0;
                getdir(dir, sizeof(dir));
                strcat(dir, "etc\\unicodetobig.txt");
                if (!ForUnicode(pU2BIG, pBIG2U, dir))
                    return 0;
            }
        }
    }
#else
    {/* This code use for Share memory Under Linux */
        int fd;
        char WrFile[heapsize];
        
        fd = open("/etc/shm_file1", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pShareGB = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file2", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pShareBIG = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file3", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pWordGB = mmap(NULL, heapsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file4", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pWordBIG = mmap(NULL, heapsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file5", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pU2GB = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file6", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pU2BIG = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file7", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pGB2U = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file8", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            pBIG2U = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        /* Unicode to Unicode */
        fd = open("/etc/shm_file9", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            UniGbBig = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file10", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            UniBigGb = mmap(NULL, buffsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file11", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            UniGbBigRule = mmap(NULL, heapsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        fd = open("/etc/shm_file12", O_RDWR | O_CREAT, 0666); 
        if (fd != -1 && write(fd, WrFile, heapsize) == heapsize)
            UniBigGbRule = mmap(NULL, heapsize, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        
        
        if (pShareGB && pShareBIG && pWordGB && pWordBIG && pU2GB && pU2BIG && pGB2U && pBIG2U \
            && UniGbBig && UniBigGb && UniGbBigRule && UniBigGbRule)
        {
            {/* Share GB and BIG5 word and rule */
                InitShareTable();
                
                memset(pWordGB, 0, heapsize);
                memset(pWordBIG, 0, heapsize);
                
                {
                    char *ptr = NULL;
                    char buf[260];
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/gb2big5.cvt2");
                    if (!InitShareFile(pWordGB, ptr, buf))
                        return 0;
                }
                {
                    char *ptr = NULL;
                    char buf[260];
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/big52gb.cvt1");
                    if (!InitShareFile(pWordBIG, ptr, buf))
                        return 0;
                }
            }
            
            {/* Share Unicode word table */
                memset(UniGbBig, 0, buffsize);
                memset(UniBigGb, 0, buffsize);
                {
                    char buf[260];
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/ungb2bigword.txt");
                    if (!InitUnicodeWord(UniGbBig, buf))
                        return 0;
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/unbig2gbword.txt");
                    if (!InitUnicodeWord(UniBigGb, buf))
                        return 0;
                }
            }
            
            {/* Share Unicode rule table */
                memset(UniGbBigRule, 0, heapsize);
                memset(UniBigGbRule, 0, heapsize);
                
                {
                    char *ptr = NULL;
                    char buf[260];
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/ungb2bigrule.txt");
                    if (!InitUnicodeRule(UniGbBigRule, ptr, buf))				
                        return 0;
                }				
                {
                    char *ptr = NULL;
                    char buf[260];
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/unbig2gbrule.txt");
                    if (!InitUnicodeRule(UniBigGbRule, ptr, buf))
                        return 0;
                }
            }
            
            {/* Share Unicode table format is &#12345; */
                memset(pU2GB, 255, buffsize);
                memset(pU2BIG, 255, buffsize);
                memset(pGB2U, 255, buffsize);
                memset(pBIG2U, 255, buffsize);
                
                {
                    char buf[260];
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/unicodetogb.txt");
                    if (!ForUnicode(pU2GB, pGB2U, buf))
                        return 0;			
                    getdir(buf, sizeof(buf));
                    strcat(buf, "etc/unicodetobig.txt");
                    if (!ForUnicode(pU2BIG, pBIG2U, buf))
                        return 0;
                }
            }
            
            {/* Forbidden convert if fjt service not started */
                key_t key;
                int shmid;				
                char *shm;
                
                key = ftok("/", 0);				
                shmid = shmget(key, 1024, IPC_CREAT|0604);
                if (shmid != -1)
                {
                    shm = (char*)shmat(shmid, 0, 0);
                    if ((int)shm != -1)									
                        strcpy(shm, "FJT service has started!\n");
                }
            }
        }
    }
#endif
    
    return(1);
}

/*-------------------------------------------------------------------------------------*/
