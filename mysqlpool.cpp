#include "context.h"
#include "mysqlpool.h"

#include <map>

using std::ios;
using std::map;
using std::to_string;

static const map<cfg, const char*>CfgStr{
    {cfg::IP, "^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"},
    {cfg::UserName, "^[-_a-zA-Z0-9]{4,16}$"},
    {cfg::PassWord, "^[A-Za-z\\d!@#$%^&*.]{8,}$"},
    {cfg::DBName, "^[a-zA-Z0-9_]+$"}
};

static const map<string, cfg>MapStr{
    {"ip", cfg::IP},
    {"username", cfg::UserName},
    {"password", cfg::PassWord},
    {"dbname", cfg::DBName},
    {"db-port", cfg::Port},
    {"db-que-size", cfg::QueSize}
};

static const map<cfg, string>GetStr{
    {cfg::IP, "ip"},
    {cfg::UserName, "username"},
    {cfg::PassWord, "password"},
    {cfg::DBName, "dbname"},
    {cfg::Port, "db-port"},
    {cfg::QueSize, "db-que-size"}
};

void DBOperator::GetInfo(Info _info){
    info=_info;
}

int DBOperator::Run(){
    *(info.Flag)=true;
    if(info.type==op::Query){
        MYSQL_RES* res=info.db->Query(info.cmd);
        MYSQL_ROW row;
        int num=mysql_num_fields(res);
        *(info.State)=true;
        if(num>0)*(info.State)=false;// 存在
    }
    if(info.type==op::Alter||info.type==op::Insert){
        if(!info.db->Update(info.cmd)){
            string _info(OperateErr+to_string(info.db->GetError()));
            info.nLog->Output(_info, level::Error);
            *(info.State)=false;
        }
    }
    return 0;
}

MysqlPool::MysqlPool(Logger* _L){
    string IP;
    unsigned short Port;
    string UserName;
    string PassWord;
    string DBName;
    string line;
    nLog=_L;
    std::ifstream file;
    file.open("config.ini", ios::in);
    if(file.fail()){
        nLog->Output(ConfigReadingErr, level::Fatal);
        throw 0;
    }
    while(getline(file, line)){
        int Idx=line.find('=');
        if(Idx==-1)continue;
        int EndIdx=line.find('\n', Idx);
        string key=line.substr(0, Idx);
        string value=line.substr(Idx+1, EndIdx-Idx-1);
        if(MapStr.find(key)==MapStr.end())continue;
        if(!IsLegal(value, MapStr.find(key)->second)){
            nLog->Output(CheckCorrectnessF+key+CheckCorrectnessB, level::Fatal);
            throw 0;
        }
        if(key=="ip")IP=value;
        else if(key=="username")UserName=value;
        else if(key=="password")PassWord=value;
        else if(key=="dbname")DBName=value;
        else if(key=="tablename")TableName=value;
        else if(key=="db-port")Port=atoi(value.c_str());
        else if(key=="db-que-size")QueueSize=atoi(value.c_str());
        else continue;
        Flag[MapStr.find(key)->second]=true;
    }
    if(TableName.empty())TableName="userinfo";
    for(auto &i:Flag)
        if(i.second==false){
            nLog->Output(CheckCorrectnessF+GetStr.find(i.first)->second+CheckCorrectnessB, level::Fatal);
            throw 0;
        }
    db=new Connection;
    Pool=new CThreadPool(QueueSize);
    if(db->Connect(IP, Port, UserName, PassWord, DBName))nLog->Output(DBConnectSuccess, level::Info);
    else{
        nLog->Output(DBConnectFatal+to_string(db->GetError()), level::Fatal);
        throw 0;
    }
    if(!db->Init(DBName, TableName)){
        nLog->Output(OperateErr+to_string(db->GetError()), level::Error);
        throw 0;
    }
}

MysqlPool::~MysqlPool(){
    Close();
}

bool MysqlPool::IsLegal(string str, cfg type){
    if(type==cfg::QueSize){
        if(atoi(str.c_str())<=0)return false;
        return true;
    }
    if(type==cfg::Port){
        if(atoi(str.c_str())<=0||atoi(str.c_str())>65535)return false;
        return true;
    }
    pattern=regex(CfgStr.find(type)->second);
    if(regex_match(str, res, pattern))return true;
    return false;
}

void MysqlPool::Close(){
    Pool->StopAll();
    nLog->Output(DBDisconnect, level::Info);
}

int MysqlPool::Operate(op _t, string _username, const char* _password, 
                       bool* _s, bool mode, bool* Flag){
    string sql;
    DBOperator* ta=new DBOperator;
    if(_t==op::Insert)
        sql="INSERT INTO "+TableName+" (username, password) VALUES ('"+_username+"', '"+_password+"')";
    if(_t==op::Query&&mode==1)
        sql="SELECT username FROM "+TableName+" WHERE username='"+_username+"'";
    if(_t==op::Query&&mode==0)
        sql="SELECT username FROM "+TableName+" WHERE username='"+_username+"' AND password='"+_password+"'";
    if(_t==op::Alter)
        sql="DELETE FROM "+TableName+" WHERE username='"+_username+"'";
    ta->GetInfo({sql, _t, nLog, db, _username.c_str(), _password, _s, mode, Flag});
    ta->SetTaskName("mysql");
    Pool->AddTask(ta);
    return 0;
}