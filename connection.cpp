/*
 * 数据库操作相关实现
 */
#include "connection.h"

using std::map;

// 获取字段名
static const map<colname,string>GetColname={
    {colname::id, "ID"},
    {colname::username, "USERNAME"},
    {colname::password, "PASSWORD"},
    {colname::create_at, "CREATE_AT"}
};

// 获取配置项
static const map<string, colname>GetColStr={
    {"ID", colname::id},
    {"USERNAME", colname::username},
    {"PASSWORD", colname::password},
    {"CREATE_AT", colname::create_at}
};

// 查询用回调函数
static int SelectCallback(void* data, int argc, char** argv, char** azColName){
    map<string, string>* Pack=reinterpret_cast<map<string, string>*>(data);
    for(int i=0;i<argc;i++){
        string ColName=string(azColName[i]);
        string Value=string(argv[i]?argv[i]:"");
        (*Pack)[ColName]=Value;
    }
    return 0;
}

// 检查表存在回调函数
static int CheckTableCallback(void* data, int argc, char** argv, char** azColName){
    bool* Flag=reinterpret_cast<bool*>(data);
    if(argc==0)(*Flag)=false;
    else (*Flag)=true;
    return 0;
}

// 检查表是否符合要求的回调函数
static int CheckLegalityCallback(void* data, int argc, char** argv, char** azColName){
    bool* Flag=reinterpret_cast<bool*>(data);
    int cnt=0;
    (*Flag)=true;
    for(int i=0;i<argc;i++){
        string tmp1=string(azColName[i]);
        string tmp2=string(argv[i]);
        if(GetColStr.find(tmp1)!=GetColStr.end())cnt++;
        if(tmp1=="USERNAME"&&tmp2!="__index")(*Flag)=false;
        if(tmp1=="PASSWORD"&&tmp2!="__index")(*Flag)=false;
    }
    if(cnt<2)(*Flag)=false;
    else if((*Flag)!=false)(*Flag)=true;
    return 0;
}

MyConnection::~MyConnection(){
    sqlite3_close(db);
}

string MyConnection::OpenDatabase(const string FileName, string TableName){
    // 打开数据库
    bool state=false;
    this->TableName=TableName;
    int res=sqlite3_open(FileName.c_str(), &db);
    if(res)return string(sqlite3_errmsg(db));
    string sql="SELECT NAME FROM sqlite_master WHERE TYPE = 'table' AND NAME = '"+TableName+"';";
    res=sqlite3_exec(db, sql.c_str(), CheckTableCallback, &state, &errMsg);
    if(res!=SQLITE_OK)return GetErr();
    if(state)return "";
    else return "Table is not exist.";
}

int MyConnection::CreateTable(){
    // 创建表
    string sql=CreateSqlF+TableName+CreateSqlB;
    int res=sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
    if(res!=SQLITE_OK)return res;
    return Insert("__index", "__index");
}

int MyConnection::Insert(const string UserName, const string Password){
    // 插入信息
    string sql="INSERT INTO "+TableName+" (USERNAME, PASSWORD) "
               "VALUES ('"+UserName+"', '"+Password+"');";
    return sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
}

int MyConnection::Select(const colname type, const string str){
    // 查询
    string sql="SELECT * FROM "+TableName+" WHERE "+GetColname.find(type)->second+" = '"+str+"';";
    return sqlite3_exec(db, sql.c_str(), SelectCallback, reinterpret_cast<void*>(&Datapack), &errMsg);
}

int MyConnection::Update(const colname type, const string str){
    // 更新信息
    string sql="UPDATE "+TableName+" SET "+GetColname.find(type)->second+" = '"+str+"';";
    return sqlite3_exec(db ,sql.c_str(), NULL, NULL, &errMsg);
}

int MyConnection::Delete(const string index){
    // 删除信息
    string sql="DELETE FROM "+TableName+" WHERE USERNAME='"+index+"';";
    return sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
}

int MyConnection::Query(const optype type, const string str1, const string str2){
    // 查询信息
    if(type==optype::_login){
        int res=Select(colname::username, str1);
        if(res!=SQLITE_OK)return res;
        if(Datapack["PASSWORD"]==str2)return 1;
        else return 0;
    }
    if(type==optype::_register){
        int res=Select(colname::username, str1);
        if(res!=SQLITE_OK)return res;
        if(Datapack["USERNAME"].empty())return 1;
        else return 0;
    }
    return 0;
}

int MyConnection::RebuildTable(){
    // 重建表
    string sql="DROP TABLE "+TableName;
    int res=sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
    if(res!=SQLITE_OK)return res;
    return CreateTable();
}

int MyConnection::CheckTable(){
    // 检查表是否符合要求
    string sql="SELECT * FROM "+TableName+" WHERE USERNAME = '__index';";
    bool state=false;
    int res=sqlite3_exec(db, sql.c_str(), CheckLegalityCallback, &state, &errMsg);
    if(res!=SQLITE_OK)return res;
    if(state==true)return 1;
    else return 0;
}

string MyConnection::GetErr(){
    return string(errMsg);
}
