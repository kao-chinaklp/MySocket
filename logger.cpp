#include "logger.h"

#include <map>
#include <regex>
#include <cstdlib>

using std::map;
using std::ios;
using std::regex;
using std::ostream;
using std::fstream;
using std::regex_replace;

using namespace logger;

static const map<level, const char*>LevelStr{
    {level::Debug, "Debug"},
    {level::Info, "info"},
    {level::Warn, "Warn"},
    {level::Error, "Error"},
    {level::Fatal, "Fatal"},
};

ostream& operator<<(ostream& stream, const string msg){
	return stream<<msg;
}

Log::LogStream Log::operator()(level nLevel){
    return LogStream(*this, nLevel);
}

string Log::GetTime(){
    string strTime;
	char str[20]={0};
	time_t tt=system_clock::to_time_t(system_clock::now());
	auto time_tm=localtime(&tt);
	sprintf(str, "%d-%02d-%02d %02d:%02d:%02d", 
        time_tm->tm_year+1900, time_tm->tm_mon+1, time_tm->tm_mday, 
        time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec);
    for(int i=0;i<19;i++)strTime.push_back(str[i]);
    return strTime;
}

void Log::EndLine(level nLevel, const char* msg){
    _lock.lock();
    Output(GetTime().c_str(), LevelStr.find(nLevel)->second, msg);
    _lock.unlock();
}

void ConsoleLogger::Output(const char* tm, const char* nLevel, const char* msg){
    printf("[%s][%s]%s\n", tm, nLevel, msg);
}

FileLogger::FileLogger(string _Time):Log(){
    string FileName(_Time+".log");
    string _FileName(FileName.size(), '\0');
    regex express("/|:| |>|<|\"|\\*|\\?|\\|");
    regex_replace(_FileName.begin(), FileName.begin(), FileName.end(), express, "-");
    _File.open(_FileName, ios::app);
    if(!_File.is_open()){
        printf("无法打开日志文件！");
        exit(0);
    }
}

FileLogger::~FileLogger(){
    _File.flush();
    _File.close();
}

void FileLogger::Output(const char* tm, const char* nLevel, const char* msg){
    _File<<"["<<tm<<"]["<<nLevel<<"]"<<msg<<std::endl;
    _File.flush();
}

int MyLog::Run(){
	ocl(nLevel)<<msg;
	ofl(nLevel)<<msg;
	return 0;
}

void MyLog::GetInfo(string _msg, level _Level){
    msg=_msg;
    nLevel=_Level;
}

Logger::Logger(int queue_size){
	QueueSize=queue_size;
	Pool=new CThreadPool(QueueSize);
	char str[20]={0};
	time_t tt=system_clock::to_time_t(system_clock::now());
	auto time_tm=localtime(&tt);
	sprintf(str, "%d-%02d-%02d %02d:%02d:%02d", 
	    time_tm->tm_year+1900, time_tm->tm_mon+1, time_tm->tm_mday, 
	    time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec);
	for(int i=0;i<19;i++)StartTime.push_back(str[i]);
}

void Logger::Output(string msg, level nLevel){
	MyLog* ta=new MyLog(StartTime);
	ta->GetInfo(msg, nLevel);
	ta->SetTaskName("log");
	Pool->AddTask(ta);
}
