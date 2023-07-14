#ifndef SERVICE_H_
#define SERVICE_H_

#include "logger.h"
#include "mysocket.h"
#include "mysqlpool.h"

class Service {
public:
  Service();
  ~Service();
  void Init();
  int Run(int type);

private:
  string DefaultConfig;
  Logger *nLog = nullptr;
  MysqlPool *db = nullptr;
  MySocket *sock = nullptr;
};

#endif
