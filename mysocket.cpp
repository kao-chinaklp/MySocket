#include "mysocket.h"
#include "context.h"

using std::ios;
using std::map;
using std::to_string;

static map<scfg, const char*>ScfgStr{
    {scfg::Cert, ""},
    {scfg::Key, ""},
    {scfg::IP, "^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"},
};

static const map<string, scfg>SmapStr{
    {"cert", scfg::Cert},
    {"key", scfg::Key},
    {"server-ip", scfg::IP},
    {"sock-port", scfg::Port},
    {"sock-que-size", scfg::QueSize}
};

static const map<scfg, string>SGetStr{
    {scfg::Cert, "cert"},
    {scfg::Key, "key"},
    {scfg::IP, "server-ip"},
    {scfg::Port, "sock-port"},
    {scfg::QueSize, "sock-que-size"}
};

void MyTask::GetSock(MySocket* socket){
    sock=socket;
}

void MyTask::GetAddr(string addr){
    this->Addr=addr;
}

int MyTask::Run(){
    if(!Acc.GetErr().empty()){
        sock->_Log(Acc.GetErr(), level::Error);
        return -1;
    }
    SSL* ssl=SSL_new(sock->GetSSL()->GetCTX());
    SSL_set_fd(ssl, connfd);
    if(SSL_accept(ssl)<=0){
        sock->_Log(ConnectFailed, level::Warn);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        #ifdef __linux__
        close(connfd);
        #else
        closesocket(connfd);
        #endif
        return 0;
    }
    sock->GetSSLList()->push_back(ssl);
    char recvBuf[1024];
    //Auth
    while(true){
        memset(recvBuf, 0x00, sizeof(recvBuf));
        if(Receive(ssl, recvBuf)<=0)return -1;
        if(!strcmp(recvBuf, login)){
            if(Receive(ssl, recvBuf, Addr)<=0)goto _close;
            string UserName(recvBuf);
            if(Receive(ssl, recvBuf, Addr)<=0)goto _close;
            string PassWord(recvBuf);
            if(!Acc.Login(UserName, PassWord)){
                sock->_Log(Acc.GetErr(), level::Info, Addr);
                SSL_write(ssl, Acc.GetErr().c_str(), 1024);
            }
            else break;
        }
        if(!strcmp(recvBuf, reg)){
            if(Receive(ssl, recvBuf, Addr)<=0)goto _close;
            string UserName(recvBuf);
            if(Receive(ssl, recvBuf, Addr)<=0)goto _close;
            string PassWord(recvBuf);
            if(!Acc.Register(UserName, PassWord)){
                sock->_Log(Acc.GetErr(), level::Info, Addr);
                SSL_write(ssl, Acc.GetErr().c_str(), 1024);
            }
            else break;
        }
    }
    //Chat
    while(true){
        memset(recvBuf, 0x00, sizeof(recvBuf));
        if(Receive(ssl, recvBuf, Acc.GetName())<=0)break;
        sock->_Log(string(recvBuf), level::Info, Acc.GetName());
        sock->SendAll(recvBuf);
    }
    for(auto i=sock->GetSSLList()->begin();i!=sock->GetSSLList()->end();i++)
        if(*i==ssl){
            sock->GetSSLList()->erase(i);
            break;
        }
    _close:
    SSL_shutdown(ssl);
    SSL_free(ssl);
    #ifdef __linux__
    close(connfd);
    #else
    closesocket(connfd);
    #endif
    return 0;
}

int MyTask::Receive(SSL* ssl, char* recv, string UserInfo){
    int len=SSL_read(ssl, recv, 1024);
    if(len==0)sock->_Log(ConnectionLost, level::Info);
    if(len<0)sock->_Log(GetErrBuf(), level::Error, UserInfo);
    return len;
}

MySocket::MySocket(Logger* _L, MysqlPool* _db){
    //init
    mLog=_L;
    db=_db;
    Init();
    //socket
    Pool=new CThreadPool(QueueSize);
    ssl=new MySSL;
    if(!ssl->init(cert, _key)){
        _Log(GetErrBuf(), level::Error);
        throw 0;
    }
    else _Log(SSLLoadSuccess, level::Info);
    #ifdef _WIN32
    WORD ver=MAKEWORD(2, 2);
    WSADATA data;
    if(WSAStartup(ver, &data)){
        _Log(CreateFailed, level::Fatal);
        throw 0;
    }
    if(LOBYTE(data.wVersion)!=2||HIBYTE(data.wHighVersion)!=2){
        _Log(VersionWrong, level::Fatal);
        throw 0;
    }
    #endif
    server_addr.sin_family=AF_INET;
    #ifdef __linux__
    inet_aton(IP.c_str(), &server_addr.sin_addr);
    #else
    server_addr.sin_addr.S_un.S_addr=inet_addr(IP.c_str());
    #endif
    server_addr.sin_port=htons(Port);
}

MySocket::~MySocket(){
    Close();
}

void MySocket::Init(){
    std::ifstream file;
    string line;
    file.open("config.ini", ios::in);
    if(file.fail()){
        _Log(ConfigReadingErr, level::Fatal);
        throw 0;
    }
    #ifdef __linux__
    ScfgStr[scfg::Cert]=ScfgStr[scfg::Key]="^[a-zA-Z0-9_\\-\\.]+\\.[a-zA-Z0-9]+$";
    #else
    ScfgStr[scfg::Cert]=ScfgStr[scfg::Key]="^[^<>:\"/\\\\|?*\\r\\n]+$";
    #endif
    while(getline(file, line)){
        int Idx=line.find('=');
        if(Idx==-1)continue;
        int EndIdx=line.find('\n', Idx);
        string key=line.substr(0, Idx);
        string value=line.substr(Idx+1, EndIdx-Idx-1);
        if(SmapStr.find(key)==SmapStr.end())continue;
        if(!IsLegal(value, SmapStr.find(key)->second)){
            _Log(CheckCorrectnessF+key+CheckCorrectnessB, level::Fatal);
            throw 0;
        }
        if(key=="cert"){
            if(value.empty())value="cacert.pem";
            cert=value;
        }
        else if(key=="key"){
            if(value.empty())value="privkey.pem";
            _key=value;
        }
        else if(key=="server-ip"){
            if(value.empty())value="127.0.0.1";
            IP=value;
        }
        else if(key=="sock-port")Port=atoi(value.c_str());
        else if(key=="sock-que-size")QueueSize=atoi(value.c_str());
        else continue;
        Flag[SmapStr.find(key)->second]=true;
    }
    for(auto &i:Flag)
        if(i.second==false){
            _Log(CheckCorrectnessF+SGetStr.find(i.first)->second+CheckCorrectnessB, level::Fatal);
            throw 0;
        }
}

void MySocket::_Log(string msg, level Level, string UserName){
    if(UserName.empty())mLog->Output(msg, Level);
    else mLog->Output("["+UserName+"] "+msg, Level);
}

void MySocket::Close(){
    #ifdef _WIN32
    WSACleanup();
    #endif
    _Log(ServiceClose, level::Info);
    ssl->Close();
    _Log(SocketClose, level::Info);
    delete ssl;
}

bool MySocket::IsLegal(string str, scfg type){
    if(str.empty()&&(type==scfg::Cert||type==scfg::Key||type==scfg::IP))return true;
    if(type==scfg::QueSize){
        if(atoi(str.c_str())<=0)return false;
        return true;
    }
    if(type==scfg::Port){
        if(atoi(str.c_str())<=0||atoi(str.c_str())>65536)return false;
        return true;
    }
    pattern=regex(ScfgStr.find(type)->second);
    if(regex_match(str, res, pattern))return true;
    return false;
}

int MySocket::Run(int type){
    //1=TCP 0=UDP
    if(type==1)server=socket(AF_INET, SOCK_STREAM, 0);
    else server=socket(AF_INET, SOCK_DGRAM, 0);
    if(bind(server, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(SOCKADDR))==SOCKET_ERROR){
        _Log(BindFatal, level::Fatal);
        throw 0;
    }
    if(listen(server, QueueSize)<0){
        _Log(ListenFatal, level::Fatal);
        throw 0;
    }
    else _Log(SuccessStartF+IP+":"+to_string(Port)+SuccessStartB, level::Info);
    int len=sizeof(SOCKADDR);
    while(true){
        s_accept=accept(server, reinterpret_cast<SOCKADDR*>(&accept_addr), &len);
        const string _IP(inet_ntoa(accept_addr.sin_addr));
        const string _Port=to_string(ntohs(accept_addr.sin_port));
        const string _Addr=_IP+":"+_Port;
        _Log(_Addr+TryConnect, level::Info);
        if(s_accept==SOCKET_ERROR){
            _Log(_Addr+ConnectFatal, level::Info);
            continue;
        }
        MyTask* ta=new MyTask;
        ta->GetSock(this);
        ta->GetAddr(_Addr);
        ta->SetConnFd(s_accept);
        ta->SetTaskName("sock"+to_string(Pool->GetTaskSize()));
        Pool->AddTask(ta);
    }
    #ifdef __linux__
    close(server);
    #else
    closesocket(server);
    #endif
    throw 0;
}

void MySocket::SendAll(char* msg){
    deque<SSL*> tmp=SSLList;
    for(auto i=tmp.begin();i!=tmp.end();i++)
        SSL_write(*i, msg, strlen(msg));
}

MySSL* MySocket::GetSSL(){
    return ssl;
}

deque<SSL*>* MySocket::GetSSLList(){
    return &SSLList;
}