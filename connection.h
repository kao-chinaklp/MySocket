#ifndef CONNECTION_H_
#define CONNECTION_H_

// 单个连接

#include "mysql.h"

#include <map>
#include <string>
#include <chrono>

using std::map;
using std::string;
using namespace std::chrono;

static const map<const char*, const char*>Require{
    {"id", "INT"},
    {"username", "VARCHAR(50)"},
    {"password", "VARCHAR(255)"},
    {"create_at", "TIMESTAMP"}
};

class Connection{
    private:
        MYSQL* Conn;
        const string CreateSqlF=
            "CREATE TABLE ";
        const string CreateSqlB=
            " (id INT AUTO_INCREMENT PRIMARY KEY,"
            "username VARCHAR(50) UNIQUE NOT NULL,"
            "password VARCHAR(255) NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    public:
        Connection();
        ~Connection();
        bool Create(string TableName);
        bool Init(string dbname, string TableName);
        int CheckTable(string dbname, string TableName);
        bool Connect(string IP, unsigned short port, string username, string psw, string dbname);
        bool Update(string sql);//增删改
        MYSQL_RES* Query(string sql);//查询
        unsigned int GetError();
};

#endif
