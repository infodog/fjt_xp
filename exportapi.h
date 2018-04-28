#ifndef  _EXPORTAPI_H_
#define  _EXPORTAPI_H_

int InitShareApi(void);
int ForUnicode(char *pFrom, char *pTo, char *pFileName);
int InitUnicodeWord(char *pbuff, char *filename);
int InitUnicodeRule(ruletable *wordlist, char *psem, char *filename);
int InitWordTable(char *ptable,char *filename);

#endif
