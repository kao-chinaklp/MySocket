#include "account.h"

Account::Account(string username, MysqlPool *_db) {
    char *err;
    UserName = username;
    db = _db;
    ERR_load_crypto_strings();
    ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        ERR_error_string(ERR_get_error(), err);
        Err = "未知错误！" + string(err);
    }
    sha256 = EVP_MD_fetch(NULL, "SHA256", NULL);
    if (ctx == nullptr) {
        ERR_error_string(ERR_get_error(), err);
        Err = "未知错误！" + string(err);
    }
}

Account::~Account() {
    OPENSSL_free(buff);
    EVP_MD_free(sha256);
    EVP_MD_CTX_free(ctx);
}

string Account::GetName() { return UserName; }

string Account::GetErr() { return Err; }

bool Account::encrypt(const char *msg) {
    unsigned int len;
    char *err;
    if (!EVP_DigestUpdate(ctx, msg, sizeof(msg))) {
        ERR_error_string(ERR_get_error(), err);
        Err = "未知错误，请联系管理员！" + string(err);
        return false;
    }
    buff = (unsigned char *)OPENSSL_malloc(EVP_MD_get_size(sha256));
    if (buff == nullptr) {
        ERR_error_string(ERR_get_error(), err);
        Err = "未知错误，请联系管理员！" + string(err);
        return false;
    }
    if (!EVP_DigestFinal_ex(ctx, buff, &len)) {
        ERR_error_string(ERR_get_error(), err);
        Err = "未知错误，请联系管理员！" + string(err);
        return false;
    }
    return true;
}

bool Account::Login(string UserName, string PassWord) {
    bool State;
    if (!encrypt(PassWord.c_str()))
        return false;
    db->Operate("select * from userinfo", op::Query, UserName,
                (const char *)(char *)buff, &State, false);
    if (State) {
        this->UserName = UserName;
        return true;
    } else {
        Err = "账号或密码错误！";
        return false;
    }
}

bool Account::Register(string UserName, string PassWord) {
    bool State;
    if (!encrypt(PassWord.c_str()))
        return false;
    db->Operate("select * from userinfo;", op::Query, UserName,
                (const char *)(char *)buff, &State, false);
    if (!State) {
        Err = "账号已经被注册！";
        return false;
    }
    db->Operate("insert into userinfo ", op::Insert, UserName, PassWord.c_str(),
                &State, true);
    if (!State) {
        Err = "未知错误，请联系管理员！";
        return false;
    } else {
        this->UserName = UserName;
        return true;
    }
}