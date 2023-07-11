#include "service.h"

#include <fstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::map;

Service::Service(){
	nLog=Logger(5);
	nLog.Output("日志系统启动成功！", level::Info);
	// init config
	DefaultConfig="# socket（密钥文件留空表示自动生成）\r\ncert=\r\nkey=\r\nsock-port=2333\r\nsock-que-size=10\r\n\r\n# mysql\r\nip=127.0.0.1\r\nusername=root\r\npassword=\r\ndbname=userinfo\r\ndb-port=3306\r\ndb-que-size=5\r\n\r\n# 版本号（请不要更改此项）\r\nversion=v0.0.1-pre";
	ifstream ifile;
	ofstream ofile;
	ifile.open("config.ini", ios::in);
	if(ifile.fail()){
		ofile.open("config.ini", ios::out);
		ofile<<DefaultConfig;
	}
	ifile.close();
	ofile.close();
	// init key&crt
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	EVP_PKEY_CTX* ctx=EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
	EVP_PKEY* pkey;
	try{
		if(!ctx){
			nLog.Output(GetErrBuf(), level::Error);
			exit(0);
		}
		if(EVP_PKEY_keygen_init(ctx)==0)throw GetErrBuf();
		if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024)<=0)throw GetErrBuf();
		if(EVP_PKEY_keygen(ctx, &pkey)<=0)throw GetErrBuf();
		nLog.Output("Debug", level::Debug);
		EVP_PKEY_CTX_free(ctx);
	}
	catch(string buf){
		nLog.Output(buf, level::Error);
		EVP_PKEY_CTX_free(ctx);
		exit(0);
	}
	string cert, _key;
	ifstream file;
    string line;
	file.open("config.ini", ios::in);
    if(file.fail()){
        nLog.Output("读取配置文件失败！", level::Fatal);
        exit(0);
    }
    while(getline(file, line)){
        int Idx=line.find('=');
        if(Idx==-1)continue;
        int EndIdx=line.find('\n', Idx);
        string key=line.substr(0, Idx);
        string value=line.substr(Idx+1, EndIdx-Idx-1);
        if(key=="cert")cert=value;
        else if(key=="key")_key=value;
    }
	file.close();
	// check key
	ifstream testcert;
	ifstream testkey;
	FILE* PublicKey=fopen(cert.c_str(), "wb");
	FILE* PrivateKey=fopen(_key.c_str(), "wb");
	testcert.open(cert);
	testkey.open(_key);
	if(!testcert.fail()&&!testkey.fail())goto nxt;
	if(!EVP_PKEY_print_public_fp(PrivateKey, pkey, 0, NULL)){
		nLog.Output(GetErrBuf(), level::Error);
		exit(0);
	}
	if(!EVP_PKEY_print_private_fp(PublicKey, pkey, 0, NULL)){
		nLog.Output(GetErrBuf(), level::Error);
		exit(0);
	}
	nxt:
	testcert.close();
	testkey.close();
	fclose(PublicKey);
	fclose(PrivateKey);
	EVP_cleanup();
	db=MysqlPool(&nLog);
	sock=MySocket(&nLog, &db);
}

int Service::Run(int type){
	sock.Run(type); 
	return 0;
}
