#include "myssl.h"

#include <string>

using std::string;

string GetErrBuf() {
    char Buf[1024];
    memset(Buf, 0, 1024);
    ERR_error_string(ERR_get_error(), Buf);
    return string(Buf);
}

bool MySSL::init(string cert, string key) {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(TLS_server_method());
    int len = SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM);
    if (ctx == nullptr ||
        SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM) <=
            0 ||
        SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0 ||
        !SSL_CTX_check_private_key(ctx))
        return false;
    return true;
}

unsigned int MySSL::GetError() { return ERR_get_error(); }

SSL_CTX *MySSL::GetCTX() { return ctx; }

void MySSL::ShutDown() {
    SSL_CTX_free(ctx);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}