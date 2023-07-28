#include "connection.h"

Connection::Connection(){
    Conn=mysql_init(nullptr);
}

Connection::~Connection(){
    if(Conn!=nullptr)mysql_close(Conn);
}

bool Connection::Create(){
    if(mysql_query(Conn, CreateSql.c_str()))return false;
    return true;
}

bool Connection::Init(string TableName){
    int state=CheckTable(TableName);
    if(state==-1)return Create();
    if(state==1)return false;
    if(state==2){
        string sql="DROP TABLE IF EXISTS "+TableName+";";
        if(mysql_query(Conn, sql.c_str()))return false;
        return Create();
    }
    return true;
}

bool Connection::Connect(string IP, unsigned short port, string username, string psw){
    MYSQL* p=mysql_real_connect(Conn, IP.c_str(), username.c_str(),
                                psw.c_str(), nullptr, port, nullptr, 0);
    return p!=nullptr;
}

bool Connection::CreateDB(string dbname){
    string sql="CREATE DATABASE IF NOT EXISTS "+dbname;
    if(mysql_query(Conn, sql.c_str()))return false;
    sql="USE "+dbname;
    if(mysql_query(Conn, sql.c_str()))return false;
    return true;
}

int Connection::CheckTable(string TableName){
    string sql="DESCRIBE "+TableName;
    if(mysql_query(Conn, sql.c_str()))return -1;// 表不存在
    MYSQL_RES* res=mysql_store_result(Conn);
    if(res==NULL)return 1;// 查询失败
    int num=mysql_num_fields(res);
    MYSQL_ROW row;
    while(row=mysql_fetch_row(res)){
        const char* name=row[0];
        const char* type=row[1];
        if(!isValidColumnType(name, type)){
            mysql_free_result(res);
            return 2;// 类型不同
        }
    }
    mysql_free_result(res);
    return 0;
}

bool Connection::isValidColumnType(const char* Column, const char* Type){
    if(strcmp(Require.find(Column)->second, Type)==0)return true;
    return false;
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