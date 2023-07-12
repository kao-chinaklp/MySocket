#ifndef MYSQLPOOL_H_
#define MYSQLPOOL_H_

#include "connection.h"
#include "logger.h"
#include "thread.h"


#include <map>
#include <regex>

using std::regex;

using namespace logger;

enum class op { Query, Insert, Update };
enum class cfg { IP, UserName, PassWord, DBName, Port, QueSize };
class DBOperator : public CTask {
public:
  class Info {
  public:
    Info() = default;
    Info(string _cmd, op _type, Logger *_Log, Connection *_db,
         const char *_username, const char *_psw, bool *_s, bool _mode)
        : cmd(_cmd), type(_type), nLog(_Log), db(_db), UserName(_username),
          PassWord(_psw), State(_s), mode(_mode) {}
    Logger *nLog;
    Connection *db;
    op type;
    string cmd;

    const char *UserName;
    const char *PassWord;
    bool *State;
    bool mode; // 0 == login 1 == register
  };

public:
  DBOperator(){};
  int Run();
  void GetInfo(Info _info);

private:
  Connection *db;
  Info info;
};

class MysqlPool {
public:
  MysqlPool() {}
  MysqlPool(Logger *_L);
  void ShutDown();
  bool IsLegal(string str, cfg type);
  int Operate(string sql, op _t, string _username, const char *_password,
              bool *_s, bool mode);

private:
  Logger *nLog;
  Connection *db;
  CThreadPool *Pool;
  regex pattern;
  std::smatch res;
  unsigned int QueueSize;
  std::map<cfg, bool> Flag;
};

#endif
