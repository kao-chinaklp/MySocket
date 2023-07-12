#include "thread.h"

void CTask::SetConnFd(int data) { connfd = data; }

int CTask::GetConnFd() { return connfd; }

void CTask::SetTaskName(const string _taskname) { TaskName = _taskname; }

string CTask::GetTaskName() { return TaskName; }

CThreadPool::CThreadPool(int threadNum) {
    shutdown = false;
    TaskNum = threadNum;
    pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
    pthreadCond = PTHREAD_COND_INITIALIZER;
    Create();
}

void *ThreadFunc(void *threadData) {
    CThreadPool *Data = (CThreadPool *)threadData;
    pthread_t tid = pthread_self();
    while (true) {
        pthread_mutex_lock(&Data->pthreadMutex);
        // 没有任务，等待执行
        while (Data->queTaskList.empty() && !Data->shutdown)
            pthread_cond_wait(&Data->pthreadCond, &Data->pthreadMutex);
        if (Data->shutdown) {
            pthread_mutex_unlock(&Data->pthreadMutex);
            pthread_exit(NULL);
        }

        CTask *task = Data->queTaskList.front();
        Data->queTaskList.pop_front();
        // 取完任务后放锁
        pthread_mutex_unlock(&Data->pthreadMutex);
        task->Run();
        delete task;
    }
    return (void *)0;
}

int CThreadPool::AddTask(CTask *task) {
    pthread_mutex_lock(&pthreadMutex);
    queTaskList.push_back(task);
    pthread_mutex_unlock(&pthreadMutex);
    // 发信号
    pthread_cond_signal(&pthreadCond);
    return 0;
}

int CThreadPool::Create() {
    pthread_id = (pthread_t *)malloc(sizeof(pthread_t) * TaskNum);
    for (int i = 0; i < TaskNum; i++)
        pthread_create(&pthread_id[i], NULL, ThreadFunc, this);
    return 0;
}

int CThreadPool::StopAll() {
    if (shutdown)
        return -1;
    shutdown = true;
    pthread_cond_broadcast(&pthreadCond);
    // 阻塞所有线程
    for (int i = 0; i < TaskNum; i++)
        pthread_join(pthread_id[i], NULL);
    free(pthread_id);
    pthread_id = NULL;
    pthread_mutex_destroy(&pthreadMutex);
    pthread_cond_destroy(&pthreadCond);
    return 0;
}

int CThreadPool::GetTaskSize() { return queTaskList.size(); }

int CThreadPool::GetConnFd(int index) {
    return queTaskList[index]->GetConnFd();
}
