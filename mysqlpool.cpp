#include "mysqlpool.h"

#include <map>

using std::ios;
using std::map;
using std::to_string;

static const map<cfg, const char *> CfgStr{
    {cfg::IP, "^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"},
    {cfg::UserName, "^[-_a-zA-Z0-9]{4,16}$"},
    {cfg::PassWord, "^[A-Za-z\\d!@#$%^&*.]{8,}$"},
    {cfg::DBName, "^[a-zA-Z0-9_]+$"}};

static const map<string, cfg> MapStr{{"ip", cfg::IP},
                                     {"username", cfg::UserName},
                                     {"password", cfg::PassWord},
                                     {"dbname", cfg::DBName},
                                     {"db-port", cfg::Port},
                                     {"db-que-size", cfg::QueSize}};

static const map<cfg, string> GetStr{{cfg::IP, "ip"},
                                     {cfg::UserName, "username"},
                                     {cfg::PassWord, "password"},
                                     {cfg::DBName, "dbname"},
                                     {cfg::Port, "db-port"},
                                     {cfg::QueSize, "db-que-size"}};

void DBOperator::GetInfo(Info _info) { info = _info; }

int DBOperator::Run() {
    if (info.type == op::Query) {
        MYSQL_RES *res = info.db->Query(info.cmd);
        MYSQL_ROW row;
        int num = mysql_num_fields(res);
        *(info.State) = true;
        while ((row = mysql_fetch_row(res)) != nullptr)
            if (strcmp(row[0], info.UserName))
                if (info.mode == 0 && strcmp(info.PassWord, row[1]) ||
                    info.mode == 1) {
                    *(info.State) = false;
                    break;
                }
    }
    if (info.type == op::Update || info.type == op::Insert) {
        if (!info.db->Update(info.cmd)) {
            string _info("无法操作数据库，错误码：" +
                         to_string(info.db->GetError()));
            info.nLog->Output(_info, level::Error);
            *(info.State) = false;
        }
    }
    return 0;
}

MysqlPool::MysqlPool(Logger *_L) {
    string IP;
    unsigned short Port;
    string UserName;
    string PassWord;
    string DBName;
    string line;
    nLog = _L;
    std::ifstream file;
    file.open("config.ini", ios::in);
    if (file.fail()) {
        nLog->Output("读取配置文件失败！", level::Fatal);
        nLog->Close();
        exit(0);
    }
    while (getline(file, line)) {
        int Idx = line.find('=');
        if (Idx == -1)
            continue;
        int EndIdx = line.find('\n', Idx);
        string key = line.substr(0, Idx);
        string value = line.substr(Idx + 1, EndIdx - Idx - 1);
        if (MapStr.find(key) == MapStr.end())
            continue;
        if (!IsLegal(value, MapStr.find(key)->second)) {
            nLog->Output("请检查 " + key + " 项是否填写正确！", level::Fatal);
            nLog->Close();
            exit(0);
        }
        if (key == "ip")
            IP = value;
        else if (key == "username")
            UserName = value;
        else if (key == "password")
            PassWord = value;
        else if (key == "dbname")
            DBName = value;
        else if (key == "db-port")
            Port = atoi(value.c_str());
        else if (key == "db-que-size")
            QueueSize = atoi(value.c_str());
        else
            continue;
        Flag[MapStr.find(key)->second] = true;
    }
    for (auto &i : Flag)
        if (i.second == false) {
            nLog->Output("请检查 " + GetStr.find(i.first)->second +
                             " 项是否填写正确！",
                         level::Fatal);
            nLog->Close();
            exit(0);
        }
    db = new Connection;
    Pool = new CThreadPool;
    if (db->Connect(IP, Port, UserName, PassWord, DBName))
        nLog->Output("数据库连接成功！", level::Info);
    else {
        nLog->Output("数据库连接失败！错误码：" + to_string(db->GetError()),
                     level::Fatal);
        nLog->Close();
        exit(0);
    }
}

bool MysqlPool::IsLegal(string str, cfg type) {
    if (type == cfg::QueSize) {
        if (atoi(str.c_str()) <= 0)
            return false;
        return true;
    }
    if (type == cfg::Port) {
        if (atoi(str.c_str()) <= 0 || atoi(str.c_str()) > 65535)
            return false;
        return true;
    }
    pattern = regex(CfgStr.find(type)->second);
    if (regex_match(str, res, pattern))
        return true;
    return false;
}

void MysqlPool::ShutDown() {
    Pool->StopAll();
    nLog->Output("数据库连接已断开。", level::Info);
}

int MysqlPool::Operate(string sql, op _t, string _username,
                       const char *_password, bool *_s, bool mode) {
    DBOperator *ta = new DBOperator;
    if (_t == op::Insert) {
        ta->GetInfo(
            {sql, _t, nLog, db, _username.c_str(), _password, _s, mode});
        ta->SetTaskName("mysql");
        Pool->AddTask(ta);
    }
    return 0;
}