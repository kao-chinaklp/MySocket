#ifndef ACCOUNT_H_
#define ACCOUNT_H_

#include "mysqlpool.h"

#include <openssl/err.h>
#include <openssl/evp.h>

class Account {
  private:
    MysqlPool *db;
    EVP_MD *sha256;
    EVP_MD_CTX *ctx;
    string Err;
    string UserName;
    unsigned char *buff;

  public:
    Account() {}
    Account(string username, MysqlPool *_db);
    ~Account();
    string GetErr();
    string GetName();
    bool encrypt(const char *msg);
    bool Login(string username, string password);
    bool Register(string username, string password);
};

#endif
