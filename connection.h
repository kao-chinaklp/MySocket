#ifndef CONNECTION_H_
#define CONNECTION_H_

// 单个连接

#include <map>
#include <chrono>
#include <string>

#include "mysql.h"

using std::map,std::string;
using namespace std::chrono;

static const map<const char*, const char*>Require{
    {"id", "INT"},
    {"username", "VARCHAR(50)"},
    {"password", "VARCHAR(255)"}
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
        unsigned int GetError();
        bool Create(string TableName);
        bool Update(string sql);//增删
        MYSQL_RES* Query(string sql);//查询
        bool Init(string dbname, string TableName);
        int CheckTable(string dbname, string TableName);
        bool Connect(string IP, unsigned short port, string username, string psw, string dbname);
};

#endif
