/*
 * 数据库连接池的声明
 * 依赖于线程池，sqlite3
 */
#ifndef MYSQLPOOL_H_
#define MYSQLPOOL_H_

#include <map>
#include <regex>

#include "logger.h"
#include "connection.h"

using std::regex;
using namespace logger;

class DBOperator:public CTask{
    public:
        class Info{
            // 操作所需要的选项及数据
            public:
                Info()=default;
                Info(optype _t, string str1, string str2, MyConnection* _db, Logger* _log, bool* _F, bool* _S):
                type(_t), username(str1), password(str2), nLog(_log), db(_db), flag(_F), state(_S){}
                optype type;
                Logger* nLog;
                MyConnection* db;
                string username;
                string password;
                bool* flag;
                bool* state;
        };

    public:
        DBOperator(){}
        int Run(); // 实现
        void GetInfo(Info _info);

    private:
        Info info;
};

class MysqlPool{
    public:
        MysqlPool(){}
        MysqlPool(Logger* _L);
        ~MysqlPool();
        void Close();
        string GetErr(); // 获取错误
        int Operate(optype _t, const string str1, const string str2, bool* Flag, bool* State);

    private:
        bool IsLegal(string str); // 键值的合法性判断
        Logger* nLog;
        regex pattern;
        std::smatch res;
        MyConnection* db;
        CThreadPool* Pool;
        // 相关数据
        string TableName;
        string DatabaseName;
        unsigned int QueueSize;
};

#endif
