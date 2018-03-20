#ifdef   WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "update.h"
#include "decompfile.h"
#include "decompdir.h"
#include "convert.h"

// #pragma  comment(lib, "advapi32.lib")

static int QuerySvcStatus(char *SvcName)
{
    int ret = 0;
    SERVICE_STATUS as;
    SC_HANDLE ap = NULL;
    SC_HANDLE scman = OpenSCManager(NULL, NULL, GENERIC_READ); 
    
    if (scman == NULL) {
        return ret;
    }
    
    while (1) {
        ap = OpenService(scman, SvcName, SERVICE_QUERY_STATUS);	
        
        if (ap == NULL) {
            break;
        }
        
        if ((ret = QueryServiceStatus(ap, &as)) == 0) {
            break;
        }	
        
        ret = (as.dwCurrentState == SERVICE_RUNNING ? 1 : 0); 
        break; 
    }
    
    CloseServiceHandle(scman);
    return ret;
}

int UpdateFJT(char *svc, char *url)
{
    char path[1024];
    char fjtpath[1024];
    char compfile[1024]; 
    
    if (QuerySvcStatus(svc) == 0) {
        return 0;
    }
    
    getdir(path, sizeof(path));
    
    {// Obtain new FJT version 		
        strcpy(compfile, path);
        strcat(compfile, "f");
        strcat(compfile, "j");
        strcat(compfile, "t");
        strcpy(fjtpath, compfile);
        my_mkdir(compfile);		
        strcat(compfile, "\\comp.wpf");
        if (pagefile(url, compfile) != 1) {
            return 0;
        }
    }
    
    {// Decompress it 
        if (decompdir(compfile, fjtpath) < 0) {
            return 0;
        } 
        unlink(compfile);
    }
    
    {// Update FJT to new version 
        PROCESS_INFORMATION ProcessInfo; 
        STARTUPINFO StartupInfo;
        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);
        
        strcat(fjtpath, "\\upgrade.exe");
        if (CreateProcess(fjtpath, path, NULL, NULL, 0, 
            CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo) == 0) {
            return 0;
        }
    }
    
    return 1;
}

#endif
