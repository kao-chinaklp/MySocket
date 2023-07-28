#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "thread.h" 

#include <mutex>
#include <chrono>
#include <sstream>
#include <fstream>

using std::mutex;
using std::ofstream;
using namespace std::chrono;

namespace logger{
    enum class level{Debug, Info, Warn, Error, Fatal, Input};
    class Log{
        // 文本缓冲区的类
        class LogStream:public std::ostringstream{
            public:
                LogStream(Log& _log, level _level):Logger(_log), nLevel(_level){};
                LogStream(const LogStream& ls):Logger(ls.Logger), nLevel(ls.nLevel){};
                ~LogStream(){
                    Logger.EndLine(nLevel, rdbuf()->str().c_str());
                }

            private:
                Log& Logger;
                level nLevel;
        };

        public:
            Log()=default;
            virtual ~Log()=default;
            virtual LogStream operator()(level nLevel=level::Info);
        
        private:
            string GetTime();
            void EndLine(level nLevel, const char* msg);
            virtual void Output(const char* tm, const char* nLevel, const char* msg)=0;

        private:
            mutex _lock;
    };

    // 控制台
    class ConsoleLogger:public Log{
        using Log::Log;
        const char Warn[5]="Warn";
        const char Error[6]="Error";
        const char Fatal[6]="Fatal";
        virtual void Output(const char* tm, const char* nLevel, const char* msg);
    };

    // 文件
    class FileLogger:public Log{
        public:
            FileLogger(string _Time);
            FileLogger(const FileLogger&)=delete;
            FileLogger(FileLogger&&)=delete;
            virtual ~FileLogger();

        private:
            virtual void Output(const char* tm, const char* nLevel, const char* msg);

        private:
            ofstream _File;
    };
    
    class MyLog:public CTask{
        public:
            MyLog(string FileName):ofl(FileLogger(FileName)){};
            void GetInfo(string _msg, level _Level=level::Info);
            int Run();

        private:
            string msg;
            level nLevel;
            FileLogger ofl;
            ConsoleLogger ocl;
    };
	
	class Logger{
		public:
			Logger(int queue_size=5);
            ~Logger();
            void Close();
			void Output(string msg, level nLevel);

		private:
			int QueueSize;
			string StartTime;
			CThreadPool* Pool;
	};
	
    //namespace logger
}

#endif
