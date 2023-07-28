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
        const string CreateSql=
            "CREATE TABLE user_accounts ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "username VARCHAR(50) UNIQUE NOT NULL,"
            "password VARCHAR(255) NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    public:
        Connection();
        ~Connection();
        bool Create();
        bool Init(string TableName);
        bool CreateDB(string dbname);
        int CheckTable(string TableName);
        bool isValidColumnType(const char* Column, const char* Type);
        bool Connect(string IP, unsigned short port, string username, string psw);
        bool Update(string sql);//增删改
        MYSQL_RES* Query(string sql);//查询
        unsigned int GetError();
};

#endif
