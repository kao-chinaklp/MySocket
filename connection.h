/*
 * 数据库操作相关声明
 */
#ifndef CONNCETION_H_
#define CONNCETION_H_

#include "sqlite3.h"

#include <map>
#include <string>

using std::string;

enum class optype{_login, _register, insert, alter, update_u, update_p};
enum class colname{id, username, password, create_at};

class MyConnection{
    public:
        MyConnection()=default;
        ~MyConnection();
        string OpenDatabase(const string FileName, const string TableName);
        int CreateTable(const string TableName);
        int Insert(const string UserName, const string PassWord);
        int Select(const colname type, const string str);
        int Update(const colname type, const string str);
        int Delete(const string index);
        int Query(const optype type, const string str1, const string str2="");
        int RebuildTable();
        string GetErr();
        int CheckTable();

    private:
        string CreateSqlF="CREATE TABLE ";
        string CreateSqlB=" (ID INT PRIMARY KEY NOT NULL, "
                          "USERNAME VARCHAR(50) UNIQUE NOT NULL, "
                          "PASSWOED VARCHAR(255) NOT NULL, "
                          "CREATE_AT TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        sqlite3* db;
        char* errMsg;
        string TableName;
        std::map<string, string>Datapack;
};

#endif