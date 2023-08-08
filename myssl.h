/*
 * ssl相关声明
 * 依赖于openssl库
 */
#ifndef MYSSL_H_
#define MYSSL_H_

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <string>

using std::string;

extern string key_psw;

string GetErrBuf();// 获取错误信息
int PasswordCallback(char* buf, int size, int flag, void* userdata);// 输入密钥密码回调函数

class MySSL{
    public:
        MySSL():ctx(nullptr){}
        ~MySSL();
        void Close();
        SSL_CTX* GetCTX();
        unsigned int GetError();
        bool init(string cert, string key);

    private:
        SSL_CTX* ctx;
};

#endif