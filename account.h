#ifndef ACCOUNT_H_
#define ACCOUNT_H_

#include <openssl/evp.h>
#include <openssl/err.h>

#include "mysqlpool.h"

class Account{
    private:
        MysqlPool* db;
        EVP_MD* sha256;
        EVP_MD_CTX* ctx;
        string Err;
        string UserName;
        unsigned char* buff;

    public:
        Account(){}
        Account(string username, MysqlPool* _db);
        ~Account();
        string GetErr();
        string GetName();
        bool Logoff(string UserName);
        bool encrypt(const char* msg);
        bool Login(string UserName, string PassWord);
        bool Register(string UserName, string PassWord);
};

#endif
