//
// Created by 郑向阳 on 2018/4/12.
//

#include "convert_utils.h"

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

int filesize(char *filename){
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    return size;
}

int readFile(unsigned char *pbuf, int nbuf, char* filename) {
    int nfile = filesize(filename);
    if (nbuf < nfile) {
        return -1;
    }
    unsigned char buf[8192];
    FILE *fp = fopen(filename, "rb");
    int n;
    unsigned char *pcur = pbuf;
    while (!feof(fp)) {
        n = fread(buf, 1, sizeof(buf), fp);
        memcpy(pcur, buf, n);
        pcur += n;
    }
    fclose(fp);
    return pcur - pbuf;
}


unsigned char * skipSpace(unsigned char *p, unsigned char *pend,int isUtf16){
    while(p < pend){
        if(isUtf16){
            unsigned char c1 = *p;
            if(p+1 >= pend){
                return pend;
            }
            unsigned char c2 = *(p+1);
            if(c1==0 && (c2=='\t' || c2=='|')){
                p+=2;
                continue;
            }
            else{
                return p;
            }

        }
        else{
            unsigned char c1 = *p;
            if(c1=='\t'){
                p++;
            }
            else{
                return p;
            }
        }

    }
    return p;
}


unsigned char * skipLineBreak(unsigned char *p, unsigned char *pend,int isUtf16){
    while(p < pend){
        if(isUtf16){
            unsigned char c1 = *p;
            if(p+1 >= pend){
                return pend;
            }
            unsigned char c2 = *(p+1);
            if(c1==0 && (c2=='\n' || c2=='\r')){
                p+=2;
                continue;
            }
            else{
                return p;
            }
        }
        else{
            unsigned char c1 = *p;
            if(c1=='\n' || c1=='\r'){
                p++;
                continue;
            }
            else{
                return p;
            }
        }

    }
    return p;
}

unsigned char * findSpace(unsigned char *p, unsigned char *pend,int isUtf16){
    while(p < pend){
        if(isUtf16){
            unsigned char c1 = *p;
            if(p+1 >= pend){
                return pend;
            }
            unsigned char c2 = *(p+1);
            if(c1==0 && (c2=='\t' || c2=='|')){
                return p;
            }
            p+=2;
        }
        else{
            unsigned char c1 = *p;
            if(c1=='\t'){
                return p;
            }
            p++;
        }
    }
    return NULL;
}

unsigned char * findLineBreak(unsigned char *p, unsigned char *pend,int isUtf16){
    while(p < pend){
        if(isUtf16){
            unsigned char c1 = *p;
            if(p+1 >= pend){
                return pend;
            }
            unsigned char c2 = *(p+1);
            if(c1==0 && (c2=='\n' || c2=='\r')){
                return p;
            }
            p+=2;
        }
        else{
            unsigned char c1 = *p;
            if(c1=='\n' || c1=='\r'){
                return p;
            }
            p++;
        }

    }
    return NULL;
}

//int ncount = 0;
void addRule(ruletable *ruletables[],unsigned char *w1, int len_w1,unsigned char *w2, int len_w2){
    int word = w1[0] * 256 + w1[1];
    if(ruletables[word]==NULL){
        ruletable *ptable = malloc(sizeof(ruletable));
        memset(ptable,0,sizeof(ruletable));
        ruletables[word] = ptable;
    }
    ruletable *ptable = ruletables[word];
    rule *r = malloc(sizeof(rule));
    ptable->num_rule++;
    r->link = ptable->body;
    ptable->body = r;
    r->lenreal = len_w1;
    r->realcode = w1;
    r->lenrep = len_w2;
    r->repcode = w2;
//    ncount++;
//    printf("%i\n",ncount);
}

unsigned char *skipBom(unsigned char *pcur){
    if(*pcur==0xFE && *(pcur+1)==0xFF){
        return pcur+2;
    }
    return pcur;
}
int initFile(ruletable *wordlist[], ruletable *reverselist[], char *filename,int isUtf16){

//    printf("initFile filename=%s, isUtf16=%i\n",filename,isUtf16);

    int nfile = filesize(filename);
    unsigned char *pcontent = malloc(nfile);
    int ncontent = readFile(pcontent,nfile,filename);

    FILE *fp;
    unsigned char *pend = pcontent + ncontent;
    unsigned char *pcur = pcontent;
    unsigned char *w1, *w2;
    int len_w1=0, len_w2=0;
    int line = 0;
    pcur = skipBom(pcur);
    while(pcur < pend){
        pcur = skipSpace(pcur,pend,isUtf16);
        if(pcur==NULL){
            break;
        }
        pcur = skipLineBreak(pcur,pend,isUtf16);
        if(pcur==NULL){
            break;
        }

        unsigned char *pendW1 = findSpace(pcur,pend,isUtf16);
        if(pendW1==NULL){
            break;
        }
        len_w1 = pendW1 - pcur;
        w1 = malloc(len_w1);
        memcpy(w1,pcur,len_w1);

        pcur = pendW1;

        pcur = skipSpace(pcur,pend,isUtf16);
        unsigned char *pendW2 = findLineBreak(pcur,pend,isUtf16);
        if(pendW2 == NULL){
            pendW2 = pend;
        }
        len_w2 = pendW2 - pcur;
        w2 = malloc(len_w2);
        memcpy(w2,pcur,len_w2);

        pcur = pendW2+2;
        pcur = skipLineBreak(pcur,pend,isUtf16);

        //找到了两个词 w1, w2,现在处理这两个词

        if(wordlist){
            line++;
            addRule(wordlist,w1,len_w1,w2,len_w2);
        }

        if(reverselist){
            addRule(reverselist,w2,len_w2,w1,len_w1);
        }
    }
    free(pcontent);
    return 1;
}

int compare_rule(const rule **r1, const rule** r2){
    int len = MIN((*r1)->lenreal,(*r2)->lenreal);
    int r = memcmp((*r1)->realcode,(*r2)->realcode,len);
    if(r!=0){
        return r;
    }
    else{
        //r==0
        return (*r2)->lenreal - (*r1)->lenreal;
    }

}

void adjust_ruletable(ruletable *wordlist[]){

    for(int i=0; i < MAX_WORD; i++){
        ruletable *ptable = wordlist[i];
        if(ptable!=NULL){
            ptable->rules = malloc(ptable->num_rule*sizeof(rule*));
            rule *prule = ptable->body;
            int j = 0;
            while(prule != NULL){
                ptable->rules[j] = prule;
                prule = prule->link;
                j++;
            }
            //然后进行排序

            qsort(ptable->rules,ptable->num_rule,sizeof(rule*),compare_rule);
        }

    }

}