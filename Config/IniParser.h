//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : @Config File Parse
//  @ File Name : IniParser.h
//  @ Date : 2011-8-22
//  @ Author : @


#if !defined(_INIFILE_H)
#define _INIFILE_H

#include <string>
#include <vector>
using namespace std;
struct IniEntry
{
        string Key;
        string Value;
        vector<string> Commnet;
};

struct IniSection
{
        string Name;			//field name
        vector<IniEntry> Entry;		//keys of field
        vector<string> Comment;
};

class IniParser
{
public:
        explicit IniParser(const string& cfgFile = "/etc/scigw/GWconfig");
        ~IniParser();

        /**
         * @note: read and parse config file, 
         */
        int Init();

        /**
         * @note: get value of key
         */
        int GetVal(const string& field, const string& key, string& val);

        /**
         * @note: set value of key
         */
        int SetVal(const string& field, const string& key, const string& val);

        /**
         * @note: get size of element of field
         */
        size_t GetSize(const string& field);
private:

        /**
         * @note: write change to config file
         */
        int Write();

        /**
         * @note: delete space in string
         */
        void Trim(string &s);

private:
        vector<IniSection> m_Sections;
        const string m_ConfigFile;
};

#endif  //_INIFILE_H
