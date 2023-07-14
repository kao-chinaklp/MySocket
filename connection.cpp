#include "connection.h"

Connection::Connection(){
    Conn=mysql_init(nullptr);
}

Connection::~Connection(){
    if(Conn!=nullptr)mysql_close(Conn);
}

bool Connection::Connect(string IP, unsigned short port, 
                         string username, string psw, string dbname){
    MYSQL* p=mysql_real_connect(Conn, IP.c_str(), username.c_str(),
                                psw.c_str(), dbname.c_str(), port, nullptr, 0);
    return p!=nullptr;
}

bool Connection::Update(string sql){
    if(mysql_query(Conn, sql.c_str()))return false;
    return true;
}

MYSQL_RES* Connection::Query(string sql){
    if(mysql_query(Conn, sql.c_str()))return nullptr;
    return mysql_use_result(Conn);
}

unsigned int Connection::GetError(){
    return mysql_errno(Conn);
}