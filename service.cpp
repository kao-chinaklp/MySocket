#include "service.h"
#include "context.h"

#include <fstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::map;

string key_psw;

int PasswordCallback(char* buf, int size, int flag, void* userdata){
	int len=key_psw.size();
	if(len<size){
		memcpy(buf, key_psw.c_str(), len);
		return len;
	}
	return 0;
}

Service::Service(){
	nLog=Logger(5);
	nLog.Output("日志系统启动成功！", level::Info);
	// init config
	ifstream ifile;
	ofstream ofile;
	ifile.open("config.ini", ios::in);
	if(ifile.fail()){
		ofile.open("config.ini", ios::out);
		ofile<<DefaultCfg;
	}
	ifile.close();
	ofile.close();
	string cert, _key;
	ifstream file;
    string line;
	file.open("config.ini", ios::in);
    if(file.fail()){
        nLog.Output("读取配置文件失败！", level::Fatal);
        nLog.Close();
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
		else if(key=="key-psw")key_psw=value;
    }
	file.close();
	if(cert.empty()||_key.empty()){
		cert="cacert.pem";
		_key="privkey.pem";
	}
	if(key_psw.empty())key_psw="123456";
	//init key&crt
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	EVP_PKEY_CTX* ctx=nullptr;
	EVP_PKEY* pkey=nullptr;
	while(true){
		FILE* PublicKey;
		FILE* PrivateKey;
		PublicKey=fopen(cert.c_str(), "r");
		PrivateKey=fopen(_key.c_str(), "r");
		if(PublicKey&&PrivateKey){
			fclose(PublicKey);
			fclose(PrivateKey);
			break;
		}
		if(PublicKey||PrivateKey){
			fclose(PublicKey?PublicKey:PrivateKey);
			nLog.Output("密钥文件不完整，正在重新生成...", level::Warn);
		}
		ctx=EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
		const EVP_CIPHER* chiper=EVP_aes_256_cfb128();
		try{
			if(!ctx)throw GetErrBuf();
			if(EVP_PKEY_keygen_init(ctx)==0)throw GetErrBuf();
			if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024)<=0)throw GetErrBuf();
			if(EVP_PKEY_keygen(ctx, &pkey)<=0)throw GetErrBuf();
			EVP_PKEY_CTX_free(ctx);
		}
		catch(string buf){
			nLog.Output(buf, level::Error);
			EVP_PKEY_CTX_free(ctx);
			exit(0);
		}
		PublicKey=fopen(cert.c_str(), "w");
		PrivateKey=fopen(_key.c_str(), "w");
		if(!PEM_write_PUBKEY(PublicKey, pkey)){
			nLog.Output(GetErrBuf(), level::Error);
			nLog.Close();
			exit(0);
		}
		if(!PEM_write_PrivateKey(PrivateKey, pkey, chiper
								, nullptr, 0, PasswordCallback, nullptr)){
			nLog.Output(GetErrBuf(), level::Error);
			nLog.Close();
			exit(0);
		}
		fclose(PublicKey);
		fclose(PrivateKey);
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(ctx);
		nLog.Output("证书成功生成！", level::Info);
	}
	db=MysqlPool(&nLog);
	sock=MySocket(&nLog, &db);
}

int Service::Run(int type){
	sock.Run(type); 
	return 0;
}
