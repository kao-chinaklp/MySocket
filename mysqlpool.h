#ifndef MYSQLPOOL_H_
#define MYSQLPOOL_H_

#include "logger.h"
#include "connection.h"

#include <map>
#include <regex>

using std::regex;

using namespace logger;

enum class op{Query, Insert, Alter};
enum class cfg{IP, UserName, PassWord, DBName, Port, QueSize};
class DBOperator:public CTask{
    public:
        class Info{
            public:
                Info()=default;
                Info(string _cmd, op _type, Logger* _Log, Connection* _db, 
                     const char* _username, const char* _psw, bool* _s, bool _mode, bool* _Flag):
                cmd(_cmd), type(_type), nLog(_Log), db(_db),
                UserName(_username), PassWord(_psw), State(_s), mode(_mode), Flag(_Flag){}
                op type;
                string cmd;
                Logger* nLog;
                Connection* db;

                bool* Flag;
                bool* State;
                bool mode; // 0 == login 1 == register
                const char* UserName;
                const char* PassWord;
        };

    public:
        DBOperator(){};
        int Run();
        void GetInfo(Info _info);

    private:
        Info info;
        Connection* db;
};

class MysqlPool{
    public:
        MysqlPool(){}
        MysqlPool(Logger* _L);
        ~MysqlPool();
        void Close();
        bool IsLegal(string str, cfg type);
        int Operate(op _t, string _username, const char* _password, bool* _s, bool mode, bool* Flag);

    private:
        Logger* nLog;
        Connection* db;
        regex pattern;
        std::smatch res;
        string TableName;
        CThreadPool* Pool;
        unsigned int QueueSize;
        std::map<cfg, bool>Flag;
};

#endif
