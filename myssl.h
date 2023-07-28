#ifndef MYSSL_H_
#define MYSSL_H_

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

using std::string;

extern string key_psw;

string GetErrBuf();
int PasswordCallback(char* buf, int size, int flag, void* userdata);

class MySSL{
    public:
        MySSL():ctx(nullptr){};
        ~MySSL();
        void Close();
        SSL_CTX* GetCTX();
        unsigned int GetError();
        bool init(string cert, string key);

    private:
        SSL_CTX* ctx;
};

#endif