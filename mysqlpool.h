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
            public:
                Info()=default;
                Info(optype _t, string str1, string str2, MyConnection* _db, Logger* _log, bool* _F, bool* _S):
                type(_t), username(str1), password(str2), nLog(_log), db(_db), flag(_F), state(_S){}
                optype type;
                string username, password;
                Logger* nLog;
                MyConnection* db;
                bool* flag;
                bool* state;
        };

    public:
        DBOperator(){}
        int Run();
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
        int Operate(optype _t, const string str1, const string str2, bool* Flag, bool* State);
        string GetErr();

    private:
        bool IsLegal(string str);
        Logger* nLog;
        MyConnection* db;
        regex pattern;
        std::smatch res;
        string TableName;
        string DatabaseName;
        CThreadPool* Pool;
        unsigned int QueueSize;
};

#endif
