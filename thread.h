#ifndef THREAD_H_
#define THREAD_H_

#include <deque>
#include <pthread.h>
#include <string>

using std::deque;
using std::string;

class CTask {
  protected:
    string TaskName;
    int connfd; // 地址

  public:
    CTask() = default;
    CTask(string &taskName) : TaskName(taskName), connfd(0) {}
    virtual int Run() = 0;
    int GetConnFd();
    void SetConnFd(int data);
    void SetTaskName(const string _taskname);
    string GetTaskName();
    virtual ~CTask() {}
};

class CThreadPool {
  private:
    deque<CTask *> queTaskList;
    pthread_mutex_t pthreadMutex; // 锁
    pthread_cond_t pthreadCond;   // 条件变量
    pthread_t *pthread_id;
    int TaskNum;
    bool shutdown;

  protected:
    friend void *ThreadFunc(void *threadData); // 回调函数
    // static int MoveToIdle(pthread_t tid);//回收
    // static int MoveToBusy(pthread_t tid);//加入忙碌队列
    int Create(); // 创建

  public:
    CThreadPool(int threadNum = 0);
    ~CThreadPool();
    int AddTask(CTask *task);
    int StopAll();
    int GetTaskSize();
    int GetConnFd(int index);
};

#endif
