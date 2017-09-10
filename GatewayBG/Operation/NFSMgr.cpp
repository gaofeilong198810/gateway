//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : NFSMgr.cpp
//  @ Date : 2011-8-3
//  @ Author : gfl
//
//

#include <sstream>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "GatewayBG/Operation/NFSMgr.h"
#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

using std::istringstream;
#define NFS_CONFIG_FILE "/etc/exports"

NFSMgr::NFSMgr()
{
}

NFSMgr::~NFSMgr()
{
}

int NFSMgr::StartNFSService()
{
        int ret;
        ChildProcessOpr cmdOpr;

        if ((ret = cmdOpr.ExecuteCmd("service nfs restart")) != 0) {
                LOG_ERROR("start nfs service error"); 
        }
        return ret;
}

int NFSMgr::StopNFSService()
{
        int ret;

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd("service nfs stop");
        if (ret != 0) {
                LOG_ERROR("stop nfs service error"); 
        }
        return ret;
}

int NFSMgr::ClearConfigFile()
{
        return truncate(NFS_CONFIG_FILE, 0);
}

int NFSMgr::GetNFSConfig(list<NfsCfgInfo>& nfsCfg)
{
        FILE *stream = fopen(NFS_CONFIG_FILE, "r");
        if (NULL == stream) {
                LOG_ERROR("open nfs config file error");
                return -1;
        }

        char   *buffer = NULL;
        size_t len;
        while (getline(&buffer, &len, stream) != -1) {
                NfsCfgInfo cfgInfo;
                string line = buffer; 
                string temp;

                if (0 == line.find('#')) {              //注释行，跳过
                        continue; 
                }
                istringstream si(line);
                si >> cfgInfo.SharedDir >> temp;
                size_t idxBegin = temp.find('(');
                size_t idxEnd = temp.find(')');
                cfgInfo.SharedIp = temp.substr(0, idxBegin); 
                cfgInfo.Permission = temp.substr(idxBegin + 1, idxEnd - idxBegin - 1);
                nfsCfg.push_back(cfgInfo); 
                buffer = NULL;
        }
        free(buffer);
        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
        }

        return 0;

}

int NFSMgr::SetNFSConfig(const string& path, const string& ip)
{
        int ret = 0;
        DirFileOpr dfOpr;  
        const string permission = "rw,sync,no_root_squash,fsid=0";
        
        if (path.empty() || ip.empty()) {
                LOG_ERROR("path, ip or permission invalid"); 
                return -1;
        }

        if (!dfOpr.HasPath(path)) {
                ret = dfOpr.MakeDir(path);
                if (ret != 0) {
                        LOG_ERROR("mkdir shared dir error"); 
                        return -2;
                }
        }

        FILE *stream = fopen(NFS_CONFIG_FILE, "a+");
        if (NULL == stream) {
                LOG_ERROR("open nfs config file for writing error"); 
                return -2;
        }

        char buffer[128];
        memset(buffer, 0, 128);
        sprintf(buffer, "%s %s(%s)\n", path.c_str(), ip.c_str(), permission.c_str());
        char *pline = NULL;
        size_t lineLen;
        while (getline(&pline, &lineLen, stream) != -1) {
                if(strcmp(buffer, pline) == 0) {
                        return 0; 
                }
        }
        free(pline);
        fprintf(stream, "%s %s(%s)\n", path.c_str(), ip.c_str(), permission.c_str());
        ret = fclose(stream);
        if (ret != 0) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int NFSMgr::GetServiceStatus(ServiceStatus& status)
{
        string line;
        size_t len;
        char   *buf = NULL;
        string cmd = "service nfs status|grep nfsd";

        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("get nfs status error");
                return -1;
        }
        if (getline(&buf, &len, stream) != -1) {
                line = buf;
                if (line.find("pid") != string::npos) {
                        status = RUNNING; 
                } else {
                        status = STOPPED; 
                }

        }
        free(buf);

        if (0 != pclose(stream)) {
                LOG_ERROR("close stream error");
        }
        return 0;	
}
