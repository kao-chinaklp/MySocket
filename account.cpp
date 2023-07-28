#include "account.h"
#include "context.h"

#include <chrono>
#include <thread>

Account::Account(string username, MysqlPool* _db){
	char* err;
	UserName=username;
	db=_db;
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
	if(!encrypt(PassWord.c_str()))
		return false;
	db->Operate(op::Query, UserName, (const char*)(char*)buff, &State, false, &Flag);
	while(!Flag)std::this_thread::sleep_for(std::chrono::milliseconds(10));
	if(State){
		this->UserName=UserName;
		return true;
	}
	else{
		Err=LoginErr;
		return false;
	}
}

bool Account::Register(string UserName, string PassWord){
	bool State, Flag=false;
	if(!encrypt(PassWord.c_str()))return false;
	db->Operate(op::Query, UserName, (const char*)(char*)buff, &State, false, &Flag);
	while(!Flag)std::this_thread::sleep_for(std::chrono::milliseconds(10));
	if(!State){
		Err=RegisterErr;
		return false;
	}
	Flag=false;
	db->Operate(op::Insert, UserName, PassWord.c_str(), &State, NULL, &Flag);
	while(!Flag)std::this_thread::sleep_for(std::chrono::milliseconds(10));
	if(!State){
		Err=AccountErr;
		return false;
	}
	this->UserName=UserName;
	return true;
}

bool Account::Logoff(string UserName){
	bool State, Flag=false;
	db->Operate(op::Alter, UserName, "", &State, NULL, &Flag);
	while(!Flag)std::this_thread::sleep_for(std::chrono::milliseconds(10));
	if(!State){
		Err=AccountErr;
		return false;
	}
	return true;
}