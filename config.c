#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"


int BLOCKLEVELSIZELOG(int size, char *file, int line)
{
	static FILE *fp = NULL;
	
/* 	// return size;
	// size = (((size) + 7) & ~7); */
	
	if (fp == NULL) {
		fp = fopen("logmem.txt", "a+");
	}
	if (fp) {
		fprintf(fp, "File: %s -- Line: %d -- Size: %d\r\n", file, line, size);
		fflush(fp);
	}
	
	return size;
}

