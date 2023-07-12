#ifndef SERVICE_H_
#define SERVICE_H_

#include "logger.h"
#include "mysocket.h"
#include "mysqlpool.h"

class Service {
  public:
    Service();
    int Run(int type);

  private:
    string DefaultConfig;
    Logger nLog;
    MysqlPool db;
    MySocket sock;
};

#endif
