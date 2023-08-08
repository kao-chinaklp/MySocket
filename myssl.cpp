/*
 * ssl相关功能实现
 */
#include "myssl.h"

string key_psw;

MySSL::~MySSL(){
    Close();
}

string GetErrBuf(){
    char Buf[1024];
    memset(Buf, 0, 1024);
    ERR_error_string(ERR_get_error(), Buf);
    return string(Buf);
}

int PasswordCallback(char* buf, int size, int flag, void* userdata){
    int len=key_psw.size();
    if(len<size){
        memcpy(buf, key_psw.c_str(), len);
        return len;
    }
    return 0;
}

bool MySSL::init(string cert, string key){
    ctx=SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_chain_file(ctx, cert.c_str());// 使用公钥证书
    // 设置证书密码
    SSL_CTX_set_default_passwd_cb(ctx, PasswordCallback);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, nullptr);
    SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM);// 使用私钥证书
    if(ctx==nullptr||!SSL_CTX_check_private_key(ctx))return false;
    return true;
}

unsigned int MySSL::GetError(){
    return ERR_get_error();
}

SSL_CTX* MySSL::GetCTX(){
    return ctx;
}

void MySSL::Close(){
    // 释放内存
    SSL_CTX_free(ctx);
}