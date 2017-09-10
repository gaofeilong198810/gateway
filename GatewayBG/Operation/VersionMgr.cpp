//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : VersionMgr.cpp
//  @ Date : 2011-8-8
//  @ Author : @gfl
//
//

#include <map>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/VersionMgr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

using std::map;
using std::cout;
using std::endl;

#define SEC_PER_MONTH   30 * 24 * 3600
const string DDFSLic = "/etc/ddfs/license/ddfs.lic";

VersionMgr::VersionMgr()
{
}

VersionMgr::~VersionMgr()
{
}

int VersionMgr::GetVerInfo(vector<string>& verInfoList, string& info)
{
        int ret;
        ret = CheckLicense("/etc/ddfs/license/ddfs.lic");
        if (LIC_INVALID == ret) {
                LOG_ERROR("invalid license");
                return -1;
        }

        FILE* file = popen("util.ddfs -license -f", "r");
        if (file == NULL) {
                LOG_ERROR("popen error! cmd= util.ddfs -f  errInfo: " << strerror(errno));
                return -2;
        }

        char*  buf = NULL;
        size_t len = 0; 
        string line;

        while (getline(&buf, &len, file) != -1) {
                line = buf;
                line.erase(line.length() - 1);
                verInfoList.push_back(line);
        }

        if (buf != NULL) {
                free(buf);
        }
        pclose(file);

        ret = LicenseInfo(info);
        if (ret < 0) {
                info.clear();
                LOG_ERROR("get license info error"); 
                return -2;
        }
        return 0;
}

int VersionMgr::ImportLicense(const string& licPath)
{
        int ret = 0;

        /* lic文件是否存在 */
        DirFileOpr dfo;
        if (!dfo.HasPath(licPath)) {
                LOG_INFO("lic not exist! licPath=" << licPath);
                return LIC_NOT_EXIST;
        }

        ret = CheckLicense(licPath);
        if (ret != 0) {
                return ret;
        }

        ret = CheckMd5(licPath);
        if (ret != 0) {
                return ret;
        }

        string destPath;
        dfo.GetParPath(DDFSLic, destPath);

        if (!dfo.HasPath(destPath)) {
                ret = dfo.MakeDir(destPath);
                if (ret < 0) {
                        LOG_ERROR("MakeDir Error! destPath=" << destPath);
                        return -1;
                }
        }

        ChildProcessOpr cpo;
        string licTmp  = "'" + licPath  + "'";
        string destTmp = "'" + destPath + "'";
        string cmd     = "mv " + licTmp + " " + destTmp;
        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
        }
        return ret;
}

int VersionMgr::ExportLicense(const string& destPath)
{
        ChildProcessOpr cpo;
        string cmd = "cp " + DDFSLic + " " + destPath;
        int ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
        }
        return ret;
}

int VersionMgr::CheckLicense(const string& licPath)
{
        string cmd = "util.ddfs l -f " + licPath;
        FILE* file = popen(cmd.c_str(), "r");
        if (file == NULL) {
                LOG_ERROR("popen error! cmd= util.ddfs -f  errInfo: " << strerror(errno));
                return -1;
        }

        const int LEN    = 256;
        char    buf[LEN];
        bool status = false;
        
        if (fgets(buf, LEN, file)) {
                if (strstr(buf, "\tVersion") == buf) {
                        status = true;
                }  
        }
        pclose(file);
        return status? 0: LIC_INVALID;
}

int VersionMgr::CheckMd5(const string& licPath)
{
        /* 源lic文件和目标lic文件 */
        string cmd = "md5sum " + licPath;
        cmd += " ";
        cmd += DDFSLic;

        FILE* file = popen(cmd.c_str(), "r");
        if (file == NULL) {
                LOG_ERROR("md5sum error! cmd=" << cmd);
                return -1;
        }

        string line;
        size_t len = 0;
        char*  buf = NULL;

        vector<string> verInfoList;
        while (getline(&buf, &len, file) != -1) {
                line = buf;
                line.erase(line.length() - 1);
                verInfoList.push_back(line);
        }
        if (buf != NULL) {
                free(buf);
        }
        pclose(file);

        string md51 = verInfoList[0].substr(0, 32);
        string md52 = verInfoList[1].substr(0, 32);
        if (md51 == md52) {
                return LIC_MD5_SAME;
        }
        return 0;
}

int VersionMgr::LicenseInfo(string& info)
{
        int ret;
        info.clear();
        ret = InitLicenseInfo(); 
        if (ret != 0) {
                LOG_ERROR("init license infomation error"); 
                return ret;
        }
        if (m_IsActive) {
                if (m_QuotaActive && (double)m_CurQuota / (double)m_AbsQuota >= 0.8) {
                        info += "quota invalid;";
                }
        } else {
                /* 数据量 */
                if (m_QuotaActive && (double)m_CurQuota / (double)m_AbsQuota >= 0.8) {
                        info += "quota invalid;";
                }
                /* 相对时间 */
                if (m_TimeActive && m_AvailTime <= SEC_PER_MONTH) {
                        info += "avail time invalid;";
                } 
                /* 绝对时间 */
                if (m_AbsTimeActive 
                        && m_AvailABSTime - time(NULL) <= SEC_PER_MONTH) {
                        info += "abs time invalid;";
                }
                /* 最后使用时间 
                if (m_LastTimeActive && m_LastAvailTime > time(NULL)) {
                        info += "last time invalid;"; 
                }
                */
        }
        return info.empty()? 0: 1;
}

int VersionMgr::InitLicenseInfo()
{
        int ret;
        FILE *stream = popen("util.ddfs l -f", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        const int LEN = 256;
        char buf[LEN]; 
        while (fgets(buf, LEN, stream)) {
                string line = buf;
                line.erase(line.find('\n'));
                //cout << line.size() << ":" << line << endl;
                if (line.find("\tActivated:") != string::npos) {
                        ret = InitBool(line , m_IsActive);
                        if (ret != 0) {
                                LOG_ERROR("init activate error"); 
                                return ret;
                        }
                } else if (line.find("\tTimeValidateEnabled:") != string::npos) {
                        ret = InitBool(line, m_TimeActive);
                        if (ret != 0) {
                                LOG_ERROR("init time status error"); 
                                return ret;
                        }
                } else if (line.find("\tQuotaValidateEnabled:") != string::npos) {
                        ret = InitBool(line, m_QuotaActive);
                        if (ret != 0) {
                                LOG_ERROR("init quota status error"); 
                                return ret;
                        }
                } else if (line.find("\tABSTimeValidateEnabled:") != string::npos) {
                        ret = InitBool(line, m_AbsTimeActive);
                        if (ret != 0) {
                                LOG_ERROR("init abstime status error"); 
                                return ret;
                        }
                } else if (line.find("\tLastAvailTimeEnabled:") != string::npos) {
                        ret = InitBool(line, m_LastTimeActive);
                        if (ret != 0) {
                                LOG_ERROR("init last time status error"); 
                                return ret;
                        }
                } else if (line.find("\tAvailTime:") != string::npos) {
                        ret = InitAvailTime(line);
                        if (ret != 0) {
                                LOG_ERROR("init time error"); 
                                return ret;
                        }
                } else if (line.find("\tLastAvailTime:") != string::npos) {
                        ret = InitTime(line, m_LastAvailTime);
                        if (ret != 0) {
                                LOG_ERROR("init last time error"); 
                                return ret;
                        }
                } else if (line.find("\tCurQuota:") != string::npos) {
                        ret = InitQuota(line, m_CurQuota);
                        if (ret != 0) {
                                LOG_ERROR("init current quota error"); 
                                return ret;
                        }
                } else if (line.find("\tABSQuota:") != string::npos) {
                        ret = InitQuota(line, m_AbsQuota);
                        if (ret != 0) {
                                LOG_ERROR("init absquota error"); 
                                return ret;
                        }
                } else if (line.find("\tAvailABSTime:") != string::npos) {
                        ret = InitTime(line, m_AvailABSTime);
                        if (ret != 0) {
                                LOG_ERROR("init deadline error"); 
                                return ret;
                        }
                } else {
                        continue; 
                }
                memset(buf, 0, LEN);
        }

        pclose(stream);
        return 0;
}

int VersionMgr::InitAvailTime(const string &info)
{
        int64_t time = 0;
        sscanf(info.c_str(), "%*s %ld %*s", &time);
        if (info.find("minute") != string::npos) {
                m_AvailTime = time * 60;
        } else if (info.find("day") != string::npos) {
                m_AvailTime = time * 86400;
        }
        //cout << "m_AvailTime=" << m_AvailTime << endl;
        return 0;
}

int VersionMgr::InitBool(const string &info, bool &member)
{
        char status = info[info.find(':') + 2];
        if (status == 'n') {
                member = false; 
        } else if (status == 'y') {
                member = true;
        } else {
                return -1; 
        }
        //cout << member << endl;
        return 0;
}

int VersionMgr::InitQuota(const string &info, int64_t &member)
{
        char unit[2] = {0};
        double size;
        sscanf(info.c_str(), "%*s %lf %s", &size, unit);
        switch (unit[0]) {
                case 'K':
                        member = int64_t(size * 1024);
                        break;
                case 'M':
                        member = int64_t(size * 1024 * 1024);
                        break;
                case 'G':
                        member = int64_t(size * 1024 * 1024 * 1024);
                        break;
                case 'T':
                        member = int64_t(size * 1024 * 1024 * 1024 * 1024);
                        break;
                case 'P':
                        member = int64_t(size * 1024 * 1024 * 1024 * 1024 * 1024);
                        break;
                default:
                        break;
        }
        //cout << member << endl;

        return 0;
}

int VersionMgr::InitTime(const string &info, int64_t &member)
{
        struct tm t;
        string tmStr = info.substr(info.find(':') + 2);
        strptime(tmStr.c_str(), "%Y-%m-%d-%H-%M-%S", &t);
        member = mktime(&t);
        //cout << m_LastAvailTime << endl;
        return 0;
}

