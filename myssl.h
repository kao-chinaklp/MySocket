#ifndef MYSSL_H_
#define MYSSL_H_

#include <openssl/ssl.h>
#include <openssl/err.h>

class MySSL{
    public:
        MySSL()=default;
        void ShutDown();
        SSL_CTX* GetCTX();
        unsigned int GetError();
        bool init(char* cert, char* key);

    private:
        SSL_CTX* ctx;
};

#endif