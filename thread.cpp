/*
 * 线程池功能的实现部分
 */
#include "thread.h"

#include <chrono>
#include <thread>

void* ThreadFunc(void* threadData){
    CThreadPool* Data=reinterpret_cast<CThreadPool*>(threadData);
    pthread_t tid=pthread_self();
    while(true){
        // 循环检测是否有任务
        pthread_mutex_lock(&Data->pthreadMutex);
        // 没有任务，等待执行
        while(Data->queTaskList.empty()&&!Data->shutdown)
            pthread_cond_wait(&Data->pthreadCond, &Data->pthreadMutex);
        if(Data->queTaskList.empty()&&Data->shutdown){
            // 线程池关闭
            pthread_mutex_unlock(&Data->pthreadMutex);
            pthread_exit(NULL);
        }
        // 得到任务
        CTask* task=Data->queTaskList.front();
        Data->queTaskList.pop_front();
        // 取完任务后放锁
        pthread_mutex_unlock(&Data->pthreadMutex);
        Data->MoveToBusy(tid);
        task->Run();
        delete task; // 释放指针
        Data->MoveToIdle(tid); // 回收线程
    }
    return reinterpret_cast<pthread_t*>(0);
}

string CTask::GetTaskName(){
    return TaskName;
}

int CTask::GetConnFd(){
    return connfd;
}

void CTask::SetConnFd(int data){
    connfd=data;
}

void CTask::SetTaskName(const string _taskname){
    TaskName=_taskname;
}

int CThreadPool::Create(){
    // 创建线程
    pthread_id=reinterpret_cast<pthread_t*>(malloc(sizeof(pthread_t)*TaskNum));
    for(int i=0;i<TaskNum;i++)
        pthread_create(&pthread_id[i], NULL, ThreadFunc, this);
    return 0;
}

void CThreadPool::Sleep(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int CThreadPool::MoveToIdle(pthread_t tid){
    deque<pthread_t>::iterator p=find(tid); // 找到对应线程
    if(p==BusyQue.end())return -1;
    pthread_mutex_lock(&pthreadMutex);
    BusyQue.erase(p); // 移出忙碌队列
    pthread_mutex_unlock(&pthreadMutex);
    return 1;
}

int CThreadPool::MoveToBusy(pthread_t tid){
    pthread_mutex_lock(&pthreadMutex);
    BusyQue.push_back(tid); // 加入忙碌队列
    pthread_mutex_unlock(&pthreadMutex);
    return 1;
}

deque<pthread_t>::iterator CThreadPool::find(pthread_t tid){
    deque<pthread_t>::iterator p;
    while(pthread_mutex_trylock(&pthreadMutex)==EBUSY) // 等待放锁
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    for(p=BusyQue.begin();p!=BusyQue.end();p++)
        if(*p==tid)break;
    pthread_mutex_unlock(&pthreadMutex);
    return p;
}

CThreadPool::CThreadPool(int threadNum){
    // 初始化
    shutdown=false;
    TaskNum=threadNum;
    pthreadMutex=PTHREAD_MUTEX_INITIALIZER;
    pthreadCond=PTHREAD_COND_INITIALIZER;
    Create();
}

CThreadPool::~CThreadPool(){
    StopAll();
    // 线程池销毁
    pthread_mutex_destroy(&pthreadMutex);
    pthread_cond_destroy(&pthreadCond);
}

int CThreadPool::StopAll(){
    if(shutdown)return -1;
    shutdown=true;
    pthread_cond_broadcast(&pthreadCond);
    deque<pthread_t>NoJoined;
    // 阻塞所有线程
    for(int i=0;i<TaskNum;i++){
        if(find(pthread_id[i])!=BusyQue.end()){
            NoJoined.push_back(pthread_id[i]);
            continue;
        }
        pthread_join(pthread_id[i], NULL);
    }
    while(!NoJoined.empty()){
        // 等待线程任务完成
        deque<pthread_t>::iterator p, tmp;
        for(p=NoJoined.begin();p!=NoJoined.end();p++){
            if(NoJoined.empty())break;
            tmp=find(*p);
            pthread_mutex_lock(&pthreadMutex);
            if(tmp==BusyQue.end()){
                pthread_join(*p, NULL);
                p=NoJoined.erase(p)-1;
            }
            pthread_mutex_unlock(&pthreadMutex);
        }
        this->Sleep(50);
    }
    free(pthread_id);
    pthread_id=NULL;
    return 0;
}

int CThreadPool::GetTaskSize(){
    return queTaskList.size();
}

int CThreadPool::GetConnFd(int index){
    return queTaskList[index]->GetConnFd();
}

int CThreadPool::AddTask(CTask *task){
    pthread_mutex_lock(&pthreadMutex);
    queTaskList.push_back(task);
    pthread_mutex_unlock(&pthreadMutex);
    // 发信号
    pthread_cond_signal(&pthreadCond);
    return 0;
}
