#include "account.h"

#include <chrono>
#include <thread>

#include "context.h"

using std::this_thread::sleep_for, std::chrono::microseconds;

Account::Account(string username, MysqlPool* _db){
    char* err;
    UserName=username;
    db=_db;
    ctx=nullptr;
    ctx=EVP_MD_CTX_new();
    if(ctx==nullptr){
        ERR_error_string(ERR_get_error(), err);
        Err=AccountErr+string(err);
    }
    sha256=EVP_MD_fetch(NULL, "SHA256", NULL);
    if(ctx==nullptr){
        ERR_error_string(ERR_get_error(), err);
        Err=AccountErr+string(err);
    }
}

Account::~Account(){
    OPENSSL_free(buff);
    EVP_MD_free(sha256);
    EVP_MD_CTX_free(ctx);
}

string Account::GetName(){
    return UserName;
}

string Account::GetErr(){
    return Err;
}

bool Account::encrypt(const char* msg){
    unsigned int len;
    char* err;
    if(!EVP_DigestUpdate(ctx, msg, sizeof(msg))){
        ERR_error_string(ERR_get_error(), err);
        Err=AccountErr+string(err);
        return false;
    }
    buff=(unsigned char*)OPENSSL_malloc(EVP_MD_get_size(sha256));
    if(buff==nullptr){
        ERR_error_string(ERR_get_error(), err);
        Err=AccountErr+string(err);
        return false;
    }
    if(!EVP_DigestFinal_ex(ctx, buff, &len)){
        ERR_error_string(ERR_get_error(), err);
        Err=AccountErr+string(err);
        return false;
    }
    return true;
}

bool Account::Login(string UserName, string PassWord){
    bool State, Flag=false;
    if(!encrypt(PassWord.c_str()))return false;
    db->Operate(optype::_login, UserName, PassWord, &Flag, &State);
    while(!Flag)sleep_for(milliseconds(10));
    if(!State){
        Err=LoginErr;
        return false;
    }
    this->UserName=UserName;
    return true;
}

bool Account::Register(string UserName, string PassWord){
    bool State, Flag=false;
    if(!encrypt(PassWord.c_str()))return false;
    db->Operate(optype::_register, UserName, PassWord, &Flag, &State);
    while(!Flag)sleep_for(milliseconds(10));
    if(!State){
        Err=RegisterErr;
        return false;
    }
    State=Flag=false;
    db->Operate(optype::insert, UserName, PassWord, &Flag, &State);
    while(!Flag)sleep_for(milliseconds(10));
    if(!State){
        Err=db->GetErr();
        return false;
    }
    this->UserName=UserName;
    return true;
}

bool Account::Logoff(){
    bool State, Flag=false;
    db->Operate(optype::alter, UserName, "", &Flag, &State);
    while(!Flag)std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if(!State){
        Err=db->GetErr();
        return false;
    }
    return true;
}

bool Account::ChangeInfo(const colname type, const string value){
    bool State, Flag=false;
    if(type==colname::username)db->Operate(optype::update_u, value, "", &Flag, &State);
    if(type==colname::password)db->Operate(optype::update_p, "", value, &Flag, &State);
    while(!Flag)std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if(!State){
        Err=db->GetErr();
        return false;
    }
    return true;
}