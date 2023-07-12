#include "mysocket.h"

using std::ios;
using std::map;
using std::to_string;

static map<scfg, const char *> ScfgStr{{scfg::Cert, ""}, {scfg::Key, ""}};

static const map<string, scfg> SmapStr{{"cert", scfg::Cert},
                                       {"key", scfg::Key},
                                       {"sock-port", scfg::Port},
                                       {"sock-que-size", scfg::QueSize}};

static const map<scfg, string> SGetStr{{scfg::Cert, "cert"},
                                       {scfg::Key, "key"},
                                       {scfg::Port, "sock-port"},
                                       {scfg::QueSize, "sock-que-size"}};

void MyTask::GetSock(MySocket *socket) { sock = socket; }

void MyTask::GetAddr(string addr) { this->Addr = addr; }

int MyTask::Run() {
  if (!Acc.GetErr().empty()) {
    sock->_Log(Acc.GetErr(), level::Error);
    return -1;
  }
  SSL *ssl = SSL_new(sock->GetSSL()->GetCTX());
  SSL_set_fd(ssl, connfd);
  if (SSL_accept(ssl) <= 0) {
    sock->_Log("连接失败！", level::Warn);
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
  // Auth
  while (true) {
    memset(recvBuf, 0x00, sizeof(recvBuf));
    if (Receive(ssl, recvBuf) <= 0)
      return -1;
    if (!strcmp(recvBuf, login)) {
      if (Receive(ssl, recvBuf, Addr) <= 0)
        goto _close;
      string UserName(recvBuf);
      if (Receive(ssl, recvBuf, Addr) <= 0)
        goto _close;
      string PassWord(recvBuf);
      if (!Acc.Login(UserName, PassWord)) {
        sock->_Log(Acc.GetErr(), level::Info, Addr);
        SSL_write(ssl, Acc.GetErr().c_str(), 1024);
      } else
        break;
    }
    if (!strcmp(recvBuf, reg)) {
      if (Receive(ssl, recvBuf, Addr) <= 0)
        goto _close;
      string UserName(recvBuf);
      if (Receive(ssl, recvBuf, Addr) <= 0)
        goto _close;
      string PassWord(recvBuf);
      if (!Acc.Register(UserName, PassWord)) {
        sock->_Log(Acc.GetErr(), level::Info, Addr);
        SSL_write(ssl, Acc.GetErr().c_str(), 1024);
      } else
        break;
    }
  }
  // Chat
  while (true) {
    memset(recvBuf, 0x00, sizeof(recvBuf));
    if (Receive(ssl, recvBuf, Acc.GetName()) <= 0)
      break;
    sock->_Log(string(recvBuf), level::Info, Acc.GetName());
    sock->SendAll(recvBuf);
  }
  for (auto i = sock->GetSSLList()->begin(); i != sock->GetSSLList()->end();
       i++)
    if (*i == ssl) {
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

int MyTask::Receive(SSL *ssl, char *recv, string UserInfo) {
  int len = SSL_read(ssl, recv, 1024);
  if (len == 0)
    sock->_Log("链接失效！", level::Info);
  if (len < 0)
    sock->_Log(GetErrBuf(), level::Error, UserInfo);
  return len;
}

MySocket::MySocket(Logger *_L, MysqlPool *_db) {
  // init
  mLog = _L;
  db = _db;
  Init();
  // socket
  WORD ver = MAKEWORD(2, 2);
  Pool = new CThreadPool(QueueSize);
  ssl = new MySSL;
  if (!ssl->init(cert, _key)) {
    _Log(GetErrBuf(), level::Error);
    this->Close();
    exit(0);
  } else
    _Log("SSL载入成功！", level::Info);
  WSADATA data;
  if (WSAStartup(ver, &data))
    _Log("创建失败！", level::Fatal);
  if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wHighVersion) != 2) {
    _Log("版本不符！", level::Fatal);
    this->Close();
    WSACleanup();
    exit(0);
  }
  server_addr.sin_family = AF_INET;
#ifdef __linux__
  inet_aton("127.0.0.1", &server_addr.sin_addr);
#else
  server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#endif
  server_addr.sin_port = htons(Port); // 端口
}

void MySocket::Init() {
  std::ifstream file;
  string line;
  file.open("config.ini", ios::in);
  if (file.fail()) {
    _Log("读取配置文件失败！", level::Fatal);
    this->Close();
    exit(0);
  }
#ifdef __linux__
  ScfgStr[scfg::Cert] = ScfgStr[scfg::Key] =
      "^[a-zA-Z0-9_\\-\\.]+\\.[a-zA-Z0-9]+$";
#else
  ScfgStr[scfg::Cert] = ScfgStr[scfg::Key] = "^[^<>:\"/\\\\|?*\\r\\n]+$";
#endif
  while (getline(file, line)) {
    int Idx = line.find('=');
    if (Idx == -1)
      continue;
    int EndIdx = line.find('\n', Idx);
    string key = line.substr(0, Idx);
    string value = line.substr(Idx + 1, EndIdx - Idx - 1);
    if (SmapStr.find(key) == SmapStr.end())
      continue;
    if (!IsLegal(value, SmapStr.find(key)->second)) {
      _Log("请检查 " + key + " 项是否填写正确！", level::Fatal);
      this->Close();
      exit(0);
    }
    if (key == "cert") {
      if (value.empty())
        value = "cacert.pem";
      cert = value;
    } else if (key == "key") {
      if (value.empty())
        value = "privkey.pem";
      _key = value;
    } else if (key == "sock-port")
      Port = atoi(value.c_str());
    else if (key == "sock-que-size")
      QueueSize = atoi(value.c_str());
    else
      continue;
    Flag[SmapStr.find(key)->second] = true;
  }
  for (auto &i : Flag)
    if (i.second == false) {
      _Log("请检查 " + SGetStr.find(i.first)->second + " 项是否填写正确！",
           level::Fatal);
      this->Close();
      exit(0);
    }
}

void MySocket::_Log(string msg, level Level, string UserName) {
  if (UserName.empty())
    mLog->Output(msg, Level);
  else
    mLog->Output("[" + UserName + "] " + msg, Level);
}

void MySocket::Close() {
  _Log("正在关闭服务。", level::Info);
  ssl->ShutDown();
  _Log("套接字已关闭。", level::Info);
  db->ShutDown();
  mLog->Close();
  delete ssl;
}

bool MySocket::IsLegal(string str, scfg type) {
  if (type == scfg::Cert && str.empty() || type == scfg::Key && str.empty())
    return true;
  if (type == scfg::QueSize) {
    if (atoi(str.c_str()) <= 0)
      return false;
    return true;
  }
  if (type == scfg::Port) {
    if (atoi(str.c_str()) <= 0 || atoi(str.c_str()) > 65536)
      return false;
    return true;
  }
  pattern = regex(ScfgStr.find(type)->second);
  if (regex_match(str, res, pattern))
    return true;
  return false;
}

int MySocket::Run(int type) {
  // 1=TCP 0=UDP
  if (type == 1)
    server = socket(AF_INET, SOCK_STREAM, 0);
  else
    server = socket(AF_INET, SOCK_DGRAM, 0);
  if (bind(server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) ==
      SOCKET_ERROR) {
    _Log("创建套接字失败！", level::Fatal);
#ifndef __linux__
    WSACleanup();
#endif
    Close();
    return -1;
  } else
    _Log("启动成功！", level::Info);
  if (listen(server, QueueSize) < 0) {
    _Log("监听失败！", level::Fatal);
#ifndef __linux__
    WSACleanup();
#endif
    Close();
    return -1;
  } else
    _Log("套接字启动成功！正在监听" + to_string(Port) + "端口", level::Info);
  int len = sizeof(SOCKADDR);
  while (true) {
    s_accept = accept(server, (SOCKADDR *)&accept_addr, &len);
    const string _IP(inet_ntoa(accept_addr.sin_addr));
    const string _Port = to_string(ntohs(accept_addr.sin_port));
    const string _Addr = _IP + ":" + _Port;
    _Log(_Addr + "尝试连接……", level::Info);
    if (s_accept == SOCKET_ERROR) {
      _Log(_Addr + " 连接失败！", level::Info);
      continue;
    }
    MyTask *ta = new MyTask;
    ta->GetSock(this);
    ta->GetAddr(_Addr);
    ta->SetConnFd(s_accept);
    ta->SetTaskName("sock" + to_string(Pool->GetTaskSize()));
    Pool->AddTask(ta);
  }
#ifdef __linux__
  close(server);
#else
  closesocket(server);
  WSACleanup();
#endif
  Close();
  return 0;
}

void MySocket::SendAll(char *msg) {
  deque<SSL *> tmp = SSLList;
  for (auto i = tmp.begin(); i != tmp.end(); i++)
    SSL_write(*i, msg, strlen(msg));
}

MySSL *MySocket::GetSSL() { return ssl; }

deque<SSL *> *MySocket::GetSSLList() { return &SSLList; }