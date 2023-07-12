#ifndef CONNECTION_H_
#define CONNECTION_H_

// 单个连接

#include "mysql.h"

#include <chrono>
#include <ctime>
#include <string>

using std::string;
using namespace std::chrono;

class Connection {
  private:
    MYSQL *Conn;
    clock_t AliveTime;

  public:
    Connection();
    ~Connection();
    bool Connect(string IP, unsigned short port, string username, string psw,
                 string dbname);
    bool Update(string sql);      // 增删改
    MYSQL_RES *Query(string sql); // 查询
    unsigned int GetError();
};

#endif
