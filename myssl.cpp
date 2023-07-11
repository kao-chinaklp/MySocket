#include "myssl.h"

#include <string>

using std::string;

string GetErrBuf(){
    char* Buf;
    ERR_error_string(ERR_get_error(), Buf);
    return string(Buf);
}

bool MySSL::init(char* cert, char* key){
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx=SSL_CTX_new(SSLv23_server_method());
    if(ctx==nullptr||
       SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM)<=0||
       SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM)<=0||
       !SSL_CTX_check_private_key(ctx))
        return false;
    return true;
}

unsigned int MySSL::GetError(){
    return ERR_get_error();
}

SSL_CTX* MySSL::GetCTX(){
    return ctx;
}

void MySSL::ShutDown(){
    SSL_CTX_free(ctx);
}