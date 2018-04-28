#include <stdio.h>
#include <stdlib.h>
#include "../convert_utils.h"

int main() {
    printf("Hello, World!\n");
    char *filename = "/Users/zhengxiangyang/work/fjt/thirdParty/dist/httpd-2.4.29/etc/gb2big5.cvt2";
    ruletable **ruletables = malloc(MAX_WORD*sizeof(ruletable*));
    ruletable **rruletables = malloc(MAX_WORD*sizeof(ruletable*));
    initFile(ruletables,rruletables,filename,0);
    adjust_ruletable(ruletables);
    adjust_ruletable(rruletables);
    return 0;
}