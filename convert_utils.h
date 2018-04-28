//
// Created by 郑向阳 on 2018/4/12.
//

#ifndef UTILS_CONVERT_UTILS_H
#define UTILS_CONVERT_UTILS_H

#define MAX_WORD  (65536)

typedef struct rule
{
    int flag;
    int lenreal;
    int lenrep;
    unsigned char *realcode;
    unsigned char *repcode;
    struct rule *link;
} rule;

typedef struct ruletable
{
    int flag;
    int num_rule;
    rule *body;
    rule **rules;
} ruletable;



int readFile(unsigned char *pbuf, int nbuf, char* filename);
int filesize(char *filename);
void adjust_ruletable(ruletable *wordlist[]);
int initFile(ruletable *wordlist[], ruletable *reverselist[], char *filename,int isUtf16);
#endif //UTILS_CONVERT_UTILS_H
