//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : VersionMgr.h
//  @ Date : 2011-8-8
//  @ Author : @gfl
//
//


#if !defined(_VERSIONMGR_H)
#define _VERSIONMGR_H

#include <string>
#include <vector>
#include "GatewayBG/Operation/BaseInfo.h"

using std::string;
using std::vector;

#define LIC_NOT_EXIST 1
#define LIC_VALID     2
#define LIC_INVALID   3
#define LIC_MD5_SAME  4

class VersionMgr
{
public:
        VersionMgr();
        ~VersionMgr();
public:
        int ImportLicense(const string& licPath);
        int ExportLicense(const string& dest);

        /*
         * @note 查看license信息，如果有提示返回info,否则info空
         */
        int GetVerInfo(vector<string>& verInfoList, string &info);
private:
        /*
         * @note 初始化license信息
         */
        int LicenseInfo(string& info);
        int InitLicenseInfo();
        int CheckMd5(const string& licPath);
        int CheckLicense(const string& licPath);
        int InitAvailTime(const string &info);
        int InitBool(const string &info, bool &member);
        int InitTime(const string &info, int64_t &member);
        int InitQuota(const string &info, int64_t &member);
private:
        bool m_IsActive;
        bool m_TimeActive;
        bool m_QuotaActive;
        bool m_AbsTimeActive;
        bool m_LastTimeActive;
        int64_t m_AvailTime;
        int64_t m_LastAvailTime;
        int64_t m_AvailABSTime;
        int64_t m_CurQuota;
        int64_t m_AbsQuota;
};

#endif  //_VERSIONMGR_H
