#ifdef   WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <direct.h>
#include <windows.h>

#include "decompfile.h"


typedef struct node {
    char dir;
    int length;
    char *name; 
    struct node *next; 
} node;

static FILE *fp = NULL;
static node *root = NULL;
static int base = 0; 
static char libbuf[8192]; 
static int nbuf = 8000;


static void InitRoot(void)
{
    int sub = (int)root - base;
    node *temp = root;
    
    while (temp->dir != -1) {
        (int)temp->name += sub;
        (int)temp->next += sub;
        temp = temp->next;
    }
}

static int do_init(FILE *fp, char *dir)
{
    int len = 0;	
    char head[64];
    char *buff = NULL;
    
    fread(head, 1, 8, fp); 
    {
        int *nh = (int*)head;
        len = *nh;
    }
    buff = (char*) malloc(len + 256);
    if (buff == NULL) {
        return -1;
    }
    fread(buff, 1, len, fp);
    {
        FILE *fh = NULL;
        char temp[1024];
        char *tf = NULL;
        strcpy(temp, dir);
        strcat(temp, "htmpXXXXXX"); 
        tf = mktemp(temp);
        if ((fh = fopen(tf, "wb")) == NULL) {
            return -1;
        }
        fwrite(buff, 1, len, fh);
        fclose(fh);
        if (DecompressFile(tf) != 1) {
            return -1;
        }
        if ((fh = fopen(tf, "rb")) == NULL) {
            return -1;
        }
        {
            fseek(fh, 0, SEEK_END);
            len = ftell(fh);
            fseek(fh, 0, SEEK_SET);
            len -= 8;
        }
        fread(head, 1, 8, fh);
        {
            int *nh = (int*)head; 
            base = *nh; 
        }
        free(buff);		
        buff = (char*) malloc(len + 256);
        if (buff == NULL) {
            return -1;
        }
        fread(buff, 1, len, fh);
        fclose(fh);
        unlink(tf);
        root = (node*)buff;
    }
    
    return 0;
}

int my_mkdir(char *path)
{
    char cmd[1024];
    char *ptr, *pend, *p = cmd;
    strcpy(cmd, path);
    pend = p + strlen(cmd); 
    {
        p = strchr(cmd, ':');
        if (p != NULL) {
            p += 2;
        }
        else {
            p = cmd;
        }
    } 
    while ((p < pend) && (ptr = strchr(p, '\\'))) {
        p = ptr + 1; 
        *ptr = '\0'; 
        mkdir(cmd); 
        strcpy(cmd, path);
    }
    mkdir(path);
    
    return 0;
}

static int ExtraFile(FILE *fp, char *dir)
{
    int sub = 0;
    char *ptr = NULL;
    char name[1024];
    node *puse = root;
    
    ptr = root->name;
    sub = strlen(ptr);
    if (*(ptr + strlen(ptr) - 1) != '\\') {
        sub++;
    }
    
    while (puse->dir != -1) {
        char *p = NULL;
        strcpy(name, dir);
        p = puse->name;
        if ((int)strlen(p) < sub) {
            puse = puse->next;
            continue;
        }
        p += sub;
        strcat(name, p);
        if (puse->dir == 1) {
            if (my_mkdir(name) == -1) {
                return -1;
            }
        }
        else {
            int len = puse->length;
            FILE *file = NULL;
            if ((file = fopen(name, "wb")) == NULL) {
                return -1;
            }
            while (len > 0) {
                int nr;
                
                if (len > nbuf) {
                    nr = nbuf;
                }
                else {
                    nr = len;
                }
                nr = fread(libbuf, 1, nr, fp);
                fwrite(libbuf, 1, nr, file);
                len -= nr;
            }
            fclose(file);
        }
        puse = puse->next;
    }
    
    return 0;
}

int decompdir(char *wpf, char *dir) 
{
    char path[1024];
    
    if (my_mkdir(dir) == -1) {
        return -1;
    }
    
    strcpy(path, dir);
    if (path[strlen(path) - 1] != '\\') {
        strcat(path, "\\");
    }
    
    if ((fp = fopen(wpf, "rb")) == NULL) {
        return -1;
    }
    
    if (do_init(fp, path) == -1) {
        return -1;
    }
    InitRoot();
    
    {
        char *pef = NULL;
        char extr[1024];
        FILE *fx = NULL;
        strcpy(extr, path);
        strcat(extr, "EtempXXXXXX");
        pef = mktemp(extr);
        if ((fx = fopen(pef, "wb")) == NULL) {
            return -1;
        }
        while (!feof(fp)) {
            int num = fread(libbuf, 1, nbuf, fp);			
            fwrite(libbuf, 1, num, fx);
        }
        fclose(fp);
        fclose(fx);
        if (DecompressFile(pef) != 1) {
            return -1;
        }
        if ((fp = fopen(pef, "rb")) == NULL) {
            return -1;
        }
        
        if (ExtraFile(fp, path) == -1) {
            return -1;
        }
        fclose(fp);
        unlink(pef);
    }
    
    return 0;
}

#endif

