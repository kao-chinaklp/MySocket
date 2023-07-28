#include "connection.h"

Connection::Connection(){
    Conn=mysql_init(nullptr);
}

Connection::~Connection(){
    if(Conn!=nullptr)mysql_close(Conn);
}

unsigned int Connection::GetError(){
    return mysql_errno(Conn);
}

bool Connection::Create(string TableName){
    string sql=CreateSqlF+TableName+CreateSqlB;
    if(mysql_query(Conn, sql.c_str()))return false;
    return true;
}

bool Connection::Update(string sql){
    if(mysql_query(Conn, sql.c_str()))return false;
    return true;
}

MYSQL_RES* Connection::Query(string sql){
    if(mysql_query(Conn, sql.c_str()))return nullptr;
    return mysql_use_result(Conn);
}

bool Connection::Init(string dbname, string TableName){
    int state=CheckTable(dbname, TableName);
    if(state==-1)return Create(TableName);
    if(state==1)return false;
    if(state==2){
        string sql="DROP TABLE IF EXISTS "+TableName+";";
        if(mysql_query(Conn, sql.c_str()))return false;
        return Create(TableName);
    }
    return true;
}

int Connection::CheckTable(string dbname, string TableName){
    string sql="SELECT COLUMN_NAME, COLUMN_TYPE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+dbname+"' AND TABLE_NAME='"+TableName+"'";
    if(mysql_query(Conn, sql.c_str()))return -1;// 表不存在
    MYSQL_RES* res=mysql_store_result(Conn);
    if(res==NULL)return 1;// 查询失败
    int num=mysql_num_fields(res);
    MYSQL_ROW row;
    bool hasId, hasUsername, hasPassword;
    hasId=hasUsername=hasPassword=false;
    while(row=mysql_fetch_row(res)){
        string name=string(row[0]);
        string type=string(row[1]);
        if(name=="id"&&type=="int")hasId=true;
        if(name=="username"&&type=="varchar")hasUsername=true;
        if(name=="password"&&type=="varchar")hasPassword=true;
    }
    mysql_free_result(res);
    return 0;
}

bool Connection::Connect(string IP, unsigned short port, string username, string psw, string dbname){
    MYSQL* p=mysql_real_connect(Conn, IP.c_str(), username.c_str(),
                                psw.c_str(), dbname.c_str(), port, nullptr, 0);
    return p!=nullptr;
}
