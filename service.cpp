#include "service.h"
#include "context.h"

#include <fstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::map;

extern string key_psw;

Service::Service(){
	nLog=new Logger(5);
	nLog->Output("日志系统启动成功！", level::Info);
}

Service::~Service(){
	if(sock!=nullptr)
		delete sock;
	if(db!=nullptr)
		delete db;
	if(nLog!=nullptr)
		delete nLog;
}

void Service::Init(){
	//检测配置文件
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
        nLog->Output("读取配置文件失败！", level::Fatal);
        throw 0;
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
	//检测证书文件
	if(cert.empty()||_key.empty()){
		cert="cacert.pem";
		_key="privkey.pem";
	}
	if(key_psw.empty())key_psw="123456";
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
			nLog->Output("密钥文件不完整，正在重新生成...", level::Warn);
		}
		ctx=EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
		const EVP_CIPHER* chiper=EVP_aes_256_cfb128();
		try{
			if(!ctx)throw GetErrBuf();
			if(EVP_PKEY_keygen_init(ctx)==0)throw GetErrBuf();
			if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024)<=0)throw GetErrBuf();
			if(EVP_PKEY_keygen(ctx, &pkey)<=0)throw GetErrBuf();
		}
		catch(string buf){
			nLog->Output(buf, level::Error);
			EVP_PKEY_CTX_free(ctx);
			throw 0;
		}
		PublicKey=fopen(cert.c_str(), "w");
		PrivateKey=fopen(_key.c_str(), "w");
		if(!PEM_write_PUBKEY(PublicKey, pkey)){
			nLog->Output(GetErrBuf(), level::Error);
			throw 0;
		}
		if(!PEM_write_PrivateKey(PrivateKey, pkey, chiper
								, nullptr, 0, PasswordCallback, nullptr)){
			nLog->Output(GetErrBuf(), level::Error);
			throw 0;
		}
		fclose(PublicKey);
		fclose(PrivateKey);
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(ctx);
		nLog->Output("证书成功生成！", level::Info);
	}
	db=new MysqlPool(nLog);
	sock=new MySocket(nLog, db);
}

int Service::Run(int type){
	sock->Run(type); 
	return 0;
}
