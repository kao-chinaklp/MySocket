#include "logger.h"

#include <map>
#include <regex>
#include <cstring>
#include <filesystem>

#include "context.h"

#define RESET "\033[0m"
#define RED "\033[31m"
#define YELLOW "\033[33m"

using std::map, std::ios, std::regex, std::regex_replace;
using std::ostream, std::fstream;

using namespace logger;
using namespace std::filesystem;

static const map<level, const char*>LevelStr{
    {level::Debug, "Debug"},
    {level::Info, "Info"},
    {level::Warn, "Warn"},
    {level::Error, "Error"},
    {level::Fatal, "Fatal"},
    {level::Input, "Input"}
};

ostream& operator<<(ostream& stream, const string msg){
    return stream<<msg;
}

Log::LogStream Log::operator()(level nLevel){
    return LogStream(*this, nLevel);
}

string Log::GetTime(){
    // 获取时间
    string strTime;
    char str[20]={0};
    time_t tt=system_clock::to_time_t(system_clock::now());
    struct tm time_tm;
    #ifdef _WIN32
    localtime_s(&time_tm, &tt);
    #else
    localtime_r(&tt, &time_tm);
    #endif
    snprintf(str, 20, "%d-%02d-%02d %02d:%02d:%02d",
        time_tm.tm_year+1900, time_tm.tm_mon+1, time_tm.tm_mday,
        time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec);
    for(int i=0;i<19;i++)strTime.push_back(str[i]);
    return strTime;
}

void Log::EndLine(level nLevel, const char* msg){
    // 输出内容
    _lock.lock();
    Output(GetTime().c_str(), LevelStr.find(nLevel)->second, msg);
    _lock.unlock();
}

void ConsoleLogger::Output(const char* tm, const char* nLevel, const char* msg){
    if(std::strcmp(nLevel, Warn)==0)printf(YELLOW);
    if(std::strcmp(nLevel, Error)==0)printf(RED);
    if(std::strcmp(nLevel, Fatal)==0)printf(RED);
    printf("[%s][%s]%s\n", tm, nLevel, msg);
    printf(RESET);
}

FileLogger::FileLogger(string _Time):Log(){
    string FileName(_Time+".log");
    string _FileName(FileName.size(), '\0');
    regex express("/|:| |>|<|\"|\\*|\\?|\\|");
    regex_replace(_FileName.begin(), FileName.begin(), FileName.end(), express, "-");
    _File.open("./logs/"+_FileName, ios::app);
    // 无法打开文件
    if(!_File.is_open()){
        printf(LogFatal);
        throw 0;
    }
}

FileLogger::~FileLogger(){
    _File.flush();
    _File.close();
}

void Logger::Close(){
    Output(LogClose, level::Info);
    Pool->StopAll();
}

void FileLogger::Output(const char* tm, const char* nLevel, const char* msg){
    _File<<"["<<tm<<"]["<<nLevel<<"]"<<msg<<std::endl;
    _File.flush();
}

int MyLog::Run(){
    if(nLevel!=level::Input)ocl(nLevel)<<msg;
    ofl(nLevel)<<msg;
    return 0;
}

void MyLog::GetInfo(string _msg, level _Level){
    msg=_msg;
    nLevel=_Level;
}

Logger::Logger(int queue_size){
    // 日志初始化
    path folder=current_path()/"logs";
    create_directory(folder);
    // 创建线程池
    QueueSize=queue_size;
    Pool=new CThreadPool(QueueSize);
    // 获取时间
    char str[20]={0};
    time_t tt=system_clock::to_time_t(system_clock::now());
    struct tm time_tm;
    #ifdef _WIN32
    localtime_s(&time_tm, &tt);
    #else
    localtime_r(&tt, &time_tm);
    #endif
    snprintf(str, 20, "%d-%02d-%02d %02d:%02d:%02d",
        time_tm.tm_year+1900, time_tm.tm_mon+1, time_tm.tm_mday,
        time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec);
    for(int i=0;i<19;i++)StartTime.push_back(str[i]);
}

Logger::~Logger(){
    Close();
}

void Logger::Output(string msg, level nLevel){
    MyLog* ta=new MyLog(StartTime);
    ta->GetInfo(msg, nLevel);
    ta->SetTaskName("log");
    Pool->AddTask(ta);
}
