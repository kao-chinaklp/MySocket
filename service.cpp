#include "service.h"

/*
 * 使用前请新建一个叫做token.h文件
 * 里面写入 #define MyToken "<your token>"
*/

#define CURL_STATICLIB

#include <curl/curl.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#ifdef __linux__
#include <unistd.h>
#endif

#include <fstream>
#include <iostream>

#include "json.hpp"
#include "token.h"
#include "context.h"

using std::ifstream;
using std::ofstream;
using std::ios;
using std::map;

extern string key_psw;

using json=nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output){
    size_t totalSize=size*nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

Service::Service(){
    nLog=new Logger(5);
    nLog->Output(LoggerStart, level::Info);
}

Service::~Service(){
    if(sock!=nullptr)delete sock;
    if(db!=nullptr)delete db;
    if(nLog!=nullptr)delete nLog;
}

void Service::Init(){
    //检测配置文件
    ifstream ifile;
    ifstream readmei;
    ofstream ofile;
    ofstream readmeo;
    ifile.open("config.ini", ios::in);
    if(ifile.fail()){
        ofile.open("config.ini", ios::out);
        ofile<<DefaultCfg<<_Version;
    }
    readmei.open("readme.txt");
    if(readmei.fail()){
        readmeo.open("readme.txt", ios::out);
        readmeo<<Readme;
    }
    if(!CheckReadme()){
        nLog->Output(ReadmeConfirm, level::Error);
        throw 0;
    }
    ifile.close();
    ofile.close();
    string cert, _key;
    ifstream file;
    string line, key, value;
    string origin;
    file.open("config.ini", ios::in);
    if(file.fail()){
        nLog->Output(ConfigReadingErr, level::Fatal);
        throw 0;
    }
    while(getline(file, line)){
        int Idx=line.find('=');
        int EndIdx;
        if(Idx==-1)goto _end;
        EndIdx=line.find('\n', Idx);
        key=line.substr(0, Idx);
        value=line.substr(Idx+1, EndIdx-Idx-1);
        if(key=="cert")cert=value;
        else if(key=="key")_key=value;
        else if(key=="key-psw")key_psw=value;
        else if(key=="version"){
            Version=value;
            origin.append("version=");
            origin.append(_Version);
            continue;
        }
        _end:
        if(line.empty())origin.append("\n");
        else origin.append(line+"\n");
    }
    file.close();
    if(CheckUpdate()){
        nLog->Output(NewVersion, level::Info);
        string state;
        while(true){
            std::getline(std::cin, state);
            nLog->Output(state, level::Input);
            if(state[0]=='Y'||state[0]=='y'){
                bool IsError=false;
                #ifdef __linux__
                if(execlp("bash MySocket-Updater", NULL)==-1)IsError=true;
                #else
                if(system("start MySocket-Updater.exe")<0)IsError=true;
                else throw 0;
                #endif
                if(IsError)nLog->Output(StartUpdaterFail+string(strerror(errno)), level::Fatal);
                break;
            }
            if(state[0]=='N'||state[0]=='n')break;
        }
    }
    if(Version<_Version){
        // 更新配置文件的版本号
        ofstream output;
        output.clear();
        output.open("config.ini");
        output<<origin;
        output.close();
    }
    //检测证书文件
    if(cert.empty()||_key.empty()){
        cert="cacert.pem";
        _key="privkey.pem";
    }
    if(key_psw.empty())key_psw="123456";
    while(true){
        // 检测证书是否完整
        FILE* Cacert=nullptr;
        FILE* PrivateKey=nullptr;
        Cacert=fopen(cert.c_str(), "r");
        PrivateKey=fopen(_key.c_str(), "r");
        if(Cacert&&PrivateKey){
            fclose(Cacert);
            fclose(PrivateKey);
            break;
        }
        if(Cacert||PrivateKey)fclose(Cacert?Cacert:PrivateKey);
        // 不完整，重新生成
        GenerateCertificate(cert, _key);
    }
    db=new MysqlPool(nLog);
    sock=new MySocket(nLog, db);
}

int Service::Run(type _type){
    sock->Run(_type);
    return 0;
}

bool Service::CheckUpdate(){
    // 检查更新
    CURL* curl;
    string UserAgent;
    string url="https://api.github.com/repos/kao-chinaklp/MySocket/releases/latest";
    string AccessToken="Authorization: token ";
    string response;
    string LatestVersion;
    AccessToken.append(MyToken);
    curl=curl_easy_init();
    UserAgent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36 Edg/115.0.1901.183";
    curl_easy_setopt(curl, CURLOPT_USERAGENT, UserAgent.c_str());// 设置头
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);// 禁用ssl，除非你有相关证书
    if(!curl){
        nLog->Output(InitCurlFail, level::Warn);
        return false;
    }
    struct curl_slist* header=nullptr;
    header=curl_slist_append(header, AccessToken.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());// 设置链接
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);// 设置github token
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res=curl_easy_perform(curl);// 获取响应
    curl_easy_cleanup(curl);
    if(res!=CURLE_OK){
        // 获取失败
        string ErrBuf(curl_easy_strerror(res));
        nLog->Output(GetUpdateFail+ErrBuf, level::Warn);
        return false;
    }
    json data=json::parse(response);
    LatestVersion=data["tag_name"];
    if(LatestVersion>_Version)return true;
    return false;
}

bool Service::CheckReadme(){
    // 检查使用前必读文件
    ifstream file;
    string line;
    file.open("readme.txt", ios::in);
    if(file.fail()){
        nLog->Output(ConfigReadingErr, level::Fatal);
        throw 0;
    }
    while(getline(file, line)){
        int Idx=line.find('=');
        if(Idx==-1)continue;
        int EndIdx=line.find('\n', Idx);
        string key=line.substr(0, Idx);
        string value=line.substr(Idx+1, EndIdx-Idx-1);
        if(key=="我已仔细阅读"&&value=="true"){
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void Service::GenerateCertificate(string cert, string _key){
    // 生成证书
    FILE* Cacert=nullptr;
    FILE* PrivateKey=nullptr;
    char* Area=new char[16];
    char* Owner=new char[64];
    ERR_load_crypto_strings();
    EVP_PKEY_CTX* ctx=nullptr;
    EVP_PKEY* pkey=nullptr;
    X509* ca=nullptr;
    X509_NAME* name=nullptr;
    //生成私钥
    ctx=EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    const EVP_CIPHER* chiper=EVP_aes_256_cfb128();
    nLog->Output(KeyfileIncomplete, level::Warn);
    try{
        if(!ctx)throw GetErrBuf();
        if(EVP_PKEY_keygen_init(ctx)==0)throw GetErrBuf();
        if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048)<=0)throw GetErrBuf();
        if(EVP_PKEY_keygen(ctx, &pkey)<=0)throw GetErrBuf();
    }
    catch(string buf){
        nLog->Output(buf, level::Error);
        EVP_PKEY_CTX_free(ctx);
        throw 0;
    }
    // 生成X509证书
    X509_set_version(ca, 2);
    // 设置持有者信息
    ca=X509_new();
    name=X509_NAME_new();
    while(true){
        printf(InputArea);
        scanf_s("%s", Area, 16);
        if(Area[0]=='\0')printf(IllegalInput);
        else break;
    }
    while(true){
        printf(InputOwner);
        scanf_s("%s", Owner, 64);
        if(Owner[0]=='\0')printf(IllegalInput);
        else break;
    }
    X509_NAME_add_entry_by_txt(name, Area, MBSTRING_ASC, (unsigned char*)Owner, -1, -1, 0);
    X509_set_subject_name(ca, name);
    X509_set_issuer_name(ca, name);
    // 设置证书有效期
    X509_gmtime_adj(X509_get_notBefore(ca), 0);
    X509_gmtime_adj(X509_get_notAfter(ca), 31536000L);// 默认一年
    X509_set_pubkey(ca, pkey);// 绑定
    ASN1_INTEGER_set(X509_get_serialNumber(ca), 1);// 设置序列号
    if(!X509_sign(ca, pkey, EVP_sha256())){
        // 签名失败
        nLog->Output(SignedFail+GetErrBuf(), level::Error);
        throw 0;
    }
    Cacert=fopen(cert.c_str(), "w");
    PrivateKey=fopen(_key.c_str(), "w");
    // 写入文件
    if(PEM_write_X509(Cacert, ca)==0){
        nLog->Output(GetErrBuf(), level::Error);
        fclose(Cacert);
        fclose(PrivateKey);
        throw 0;
    }
    if(!PEM_write_PrivateKey(PrivateKey, pkey, chiper
                            , nullptr, 0, PasswordCallback, nullptr)){
        nLog->Output(GetErrBuf(), level::Error);
        fclose(Cacert);
        fclose(PrivateKey);
        throw 0;
    }
    fclose(Cacert);
    fclose(PrivateKey);
    X509_free(ca);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    nLog->Output(KeyfileWSuccess, level::Info);
}
