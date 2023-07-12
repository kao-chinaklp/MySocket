#ifndef MYSSL_H_
#define MYSSL_H_

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

using std::string;

string GetErrBuf();

class MySSL {
  public:
    MySSL() = default;
    void ShutDown();
    SSL_CTX *GetCTX();
    unsigned int GetError();
    bool init(string cert, string key);

  private:
    SSL_CTX *ctx;
};

#endif