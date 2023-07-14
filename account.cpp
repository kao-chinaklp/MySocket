#include "account.h"
#include "context.h"

Account::Account(string username, MysqlPool* _db){
	char* err;
	UserName=username;
	db=_db;
	ERR_load_crypto_strings();
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
	bool State;
	if(!encrypt(PassWord.c_str()))
		return false;
	db->Operate("select * from userinfo", op::Query, 
			    UserName, (const char*)(char*)buff, &State, false);
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
	bool State;
	if(!encrypt(PassWord.c_str()))
		return false;
	db->Operate("select * from userinfo;", op::Query, 
			    UserName, (const char*)(char*)buff, &State, false);
	if(!State){
		Err=RegisterErr;
		return false;
	}
	db->Operate("insert into userinfo ", op::Insert, UserName, PassWord.c_str(), &State, true);
	if(!State){
		Err=AccountErr;
		return false;
	}
	else{
		this->UserName=UserName;
		return true;
	}
}