#ifndef THREAD_H_
#define THREAD_H_

#include <deque>
#include <string>
#include <pthread.h>

using std::string;
using std::deque;

class CTask{
    protected:
        string TaskName;
        int connfd;//地址
    
    public:
        CTask()=default;
        CTask(string &taskName):TaskName(taskName), connfd(0){}
        virtual int Run()=0;
        int GetConnFd();
        void SetConnFd(int data);
        void SetTaskName(const string _taskname);
        string GetTaskName();
        virtual ~CTask(){}
};

class CThreadPool{
    private:
        deque<CTask*> queTaskList;
        deque<pthread_t>BusyQue;//忙碌队列
        pthread_mutex_t pthreadMutex;//锁
        pthread_cond_t pthreadCond;//条件变量
        pthread_t *pthread_id;
        int TaskNum;
        bool shutdown;
    
    protected:
        friend void* ThreadFunc(void* threadData);//回调函数
        int MoveToIdle(pthread_t tid);//回收
        int MoveToBusy(pthread_t tid);//加入忙碌队列
        int Create();//创建
        void Sleep(int ms);
        deque<pthread_t>::iterator find(pthread_t tid);

    public:
        CThreadPool(int threadNum=0);
        ~CThreadPool();
        int AddTask(CTask* task);
        int StopAll();
        int GetTaskSize();
        int GetConnFd(int index);
};

#endif
