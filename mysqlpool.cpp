#include "mysqlpool.h"

#include <map>
#include <iostream>

#include "context.h"

using std::ios;

void DBOperator::GetInfo(Info _info){
    info=_info;
}

int DBOperator::Run(){
    int res;
    if(info.type==optype::_login){
        res=info.db->Query(info.type, info.username, info.password);
        if(res==1)(*info.state)=true;
        if(res==0)(*info.state)=false;
        if(res==-1)info.nLog->Output(info.db->GetErr(), level::Error);
    }
    if(info.type==optype::_register){
        res=info.db->Query(info.type, info.username);
        if(res==1)(*info.state)=true;
        if(res==0)(*info.state)=false;
        if(res==-1)info.nLog->Output(info.db->GetErr(), level::Error);
    }
    if(info.type==optype::insert){
        res=info.db->Insert(info.username, info.password);
        if(res!=SQLITE_OK)info.nLog->Output(info.db->GetErr(), level::Error);
        else (*info.state)=true;
    }
    if(info.type==optype::alter){
        res=info.db->Delete(info.username);
        if(res!=SQLITE_OK)info.nLog->Output(info.db->GetErr(), level::Error);
        else (*info.state)=true;
    }
    (*info.flag)=true;
    return 0;
}

MysqlPool::MysqlPool(Logger* _L){
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
        if(key=="db-que-size"&&atoi(value.c_str())<=0||
           key=="database"&&!IsLegal(value)){
            nLog->Output(CheckCorrectnessF+key+CheckCorrectnessB, level::Fatal);
            throw 0;
        }
        if(key=="tablename")TableName=value;
        else if(key=="database")DatabaseName=value;
        else if(key=="db-que-size")QueueSize=atoi(value.c_str());
        else continue;
    }
    if(TableName.empty())TableName="userinfo";
    if(DatabaseName.empty())DatabaseName="MySocket.db";
    db=new MyConnection;
    Pool=new CThreadPool(QueueSize);
    string buf;
    buf=db->OpenDatabase(DatabaseName, TableName);
    if(buf.empty())nLog->Output(DBConnectSuccess, level::Info);
    else if(buf=="Table is not exist.")
            if(db->CreateTable(TableName)!=SQLITE_OK){
                nLog->Output(TableCreateFailed+db->GetErr(), level::Error);
                throw 0;
            }
        else {
            nLog->Output(DBConnectFatal+buf, level::Fatal);
            throw 0;
        }
    int rs=db->CheckTable();
    if(rs==-1){
        nLog->Output(db->GetErr(), level::Error);
        throw 0;
    }
    if(rs==0){
        nLog->Output(TableIllegal, level::Warn);
        string choise;
        getline(std::cin, choise);
        if(choise[0]=='n'||choise[0]=='N')throw 0;
        if(choise[0]=='y'||choise[0]=='Y'){
            if(db->RebuildTable()!=SQLITE_OK){
                nLog->Output(TableRebuildFailed, level::Fatal);
                throw 0;
            }
            nLog->Output(TableRebuildSuccess, level::Info);
        }
    }
    nLog->Output(DatabaseInitFinished, level::Info);
}

MysqlPool::~MysqlPool(){
    Close();
}

void MysqlPool::Close(){
    Pool->StopAll();
    nLog->Output(DBDisconnect, level::Info);
}

int MysqlPool::Operate(optype _t, const string str1, const string str2, bool* Flag, bool* State){
    DBOperator* ta=new DBOperator;
    ta->GetInfo({_t, str1, str2, db, nLog, Flag, State});
    ta->SetTaskName("mysql");
    Pool->AddTask(ta);
    return 0;
}

string MysqlPool::GetErr(){
    return db->GetErr();
}

bool MysqlPool::IsLegal(string str){
    if(str.empty())return true;
    string rule;
    #ifdef __linux__
    rule="^[a-zA-Z0-9_\\-\\.]+\\.[a-zA-Z0-9]+$";
    #else
    rule="^(?![\\/:*?\"<>|]).{1,255}$";
    #endif
    pattern=regex(rule);
    if(regex_match(str, res, pattern))return true;
    return false;
}