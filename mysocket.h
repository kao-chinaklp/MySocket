/*
 * MySocket相关声明
 * 依赖于线程池、日志系统、数据库连接池
 */
#ifndef MYSOCKET_H_
#define MYSOCKET_H_

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "myssl.h"
#include "account.h"

using namespace logger;

enum class mode{ipv4, ipv6, unknown};
enum class type{tcp, udp};
enum class scfg{Cert, Key, IP, Port, QueSize, Mode};
class MySocket;

class MyTask:public CTask{
    protected:
        SSL* ssl;
        string Addr;
        Account* Acc;
        MySocket* sock;
        // 账号操作相关字符串
        const char login[6]="login";
        const char reg[9]="register";
        const char logoff[7]="logoff";
        const char logout[7]="logout";
        const char update_u[9]="update_u";
        const char update_p[9]="update_p";

    public:
        MyTask();
        int Run();
        void GetAddr(string addr);
        void GetSock(MySocket* socket);
        int Receive(SSL* ssl, char* recv, string UserInfo="");// 接收信息
};

class MySocket{
    private:
        MySSL* ssl;
        Logger* mLog;// 日志系统
        MysqlPool* db;// 数据库连接池
        SOCKET server;
        SOCKET s_accept;
        CThreadPool* Pool;// 线程池
        deque<SSL*> SSLList;
        SOCKADDR_IN server_addr;
        SOCKADDR_IN accept_addr;
        SOCKADDR_IN6 v6_server_addr;
        SOCKADDR_IN6 v6_accept_addr;
        WORD ver;
        int Port;
        string IP;
        mode _mode;
        string cert;
        string _key;
        WSADATA data;
        int QueueSize;
        regex pattern;
        std::smatch res;
        std::map<scfg, bool>Flag;
        std::map<string, bool>UserList;

    protected:
        void Initialize();// 初始化
        void SendAll(char* msg);
        void Close();
        bool IsLegal(string str, scfg type);

    public:
        MySocket()=default;
        MySocket(Logger* _L, MysqlPool* _db);
        ~MySocket();
        void v4mode(type _type);
        void v6mode(type _type);
        MySSL* GetSSL();
        MysqlPool* GetDatabase();
        int Run(type _type);
        deque<SSL*>* GetSSLList();
        void AddUser(string username);
        bool QueryUser(string username);
        void DeleteUser(string username);
        void _Log(string msg, level Level=level::Info, string UserName="");
};

#endif
