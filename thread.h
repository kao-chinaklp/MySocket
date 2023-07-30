#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>

#include <deque>
#include <string>

using std::deque;
using std::string;

class CTask{
    protected:
        string TaskName;
        int connfd;//地址

    public:
        CTask()=default;
        CTask(string &taskName):TaskName(taskName), connfd(0){}
        virtual ~CTask(){}
        int GetConnFd();
        virtual int Run()=0;
        string GetTaskName();
        void SetConnFd(int data);
        void SetTaskName(const string _taskname);
};

class CThreadPool{
    private:
        int TaskNum;
        bool shutdown;
        pthread_t *pthread_id;
        deque<CTask*> queTaskList;
        deque<pthread_t>BusyQue;//忙碌队列
        pthread_mutex_t pthreadMutex;//锁
        pthread_cond_t pthreadCond;//条件变量

    protected:
        int Create();//创建
        void Sleep(int ms);
        int MoveToIdle(pthread_t tid);//回收
        int MoveToBusy(pthread_t tid);//加入忙碌队列
        deque<pthread_t>::iterator find(pthread_t tid);
        friend void* ThreadFunc(void* threadData);//回调函数

    public:
        CThreadPool(int threadNum=0);
        ~CThreadPool();
        int StopAll();
        int GetTaskSize();
        int GetConnFd(int index);
        int AddTask(CTask* task);
};

#endif
