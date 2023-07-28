#ifndef MYSOCKET_H_
#define MYSOCKET_H_

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#include "myssl.h"
#include "logger.h"
#include "thread.h"
#include "account.h"
#include "mysqlpool.h"

using namespace logger;

enum class scfg{Cert, Key, IP, Port, QueSize};
class MySocket;

class MyTask:public CTask{
    protected:
        SSL* ssl;
        string Addr;
        Account Acc;
        MySocket* sock;
        const char login[6]="login";
        const char reg[9]="register";
    
    public:
        MyTask(){};
        int Run();
        void GetAddr(string addr);
        void GetSock(MySocket* socket);
        int Receive(SSL* ssl, char* recv, string UserInfo="");
};

class MySocket{
    private:
        MySSL* ssl;
        Logger* mLog;
        MysqlPool* db;
        SOCKET server;
        SOCKET s_accept;
        CThreadPool* Pool;
        deque<SSL*> SSLList;
        SOCKADDR_IN server_addr;
        SOCKADDR_IN accept_addr;
        int Port;
        string IP;
        string cert;
        string _key;
        int QueueSize;
        regex pattern;
        std::smatch res;
        std::map<scfg, bool>Flag;

    public:
        MySocket()=default;
        MySocket(Logger* _L, MysqlPool* _db);
        ~MySocket();
        void Init();
        void Close();
        MySSL* GetSSL();
        int Run(int type);
        void SendAll(char* msg);
        deque<SSL*>* GetSSLList();
        bool IsLegal(string str, scfg type);
        void _Log(string msg, level Level=level::Info, string UserName="");
};

#endif
