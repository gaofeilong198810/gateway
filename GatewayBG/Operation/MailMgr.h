//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : MailMgr.h
//  @ Date : 2011-8-4
//  @ Author : @gfl
//
//


#if !defined(_MAILMGR_H)
#define _MAILMGR_H
#include <string>
using std::string;

class MailMgr
{
public:
        MailMgr();
        ~MailMgr();

public:
        /**
         * @note 将邮件配置信息写入文件
         */
        int SetMailInfo(const string& ip, const string& pop3, const string& smtp);

        /**
         * @note 从文件中读取邮件配置信息
         */
        int GetMailInfo(string& ip, string& pop3, string& smtp);

private:
        const string m_MailCfgFilePath;         //邮件配置文件路径
};

#endif  //_MAILMGR_H
