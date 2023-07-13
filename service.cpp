#include "service.h"
#include "util.h"

#include <fstream>
#include <openssl/pem.h>
#include <openssl/rsa.h>

using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;

Service::Service() {
    nLog = Logger(5);
    nLog.Output("日志系统启动成功！", level::Info);
    // init config
    DefaultConfig =
        "# "
        "socket（密钥文件留空表示自动生成）\ncert=\nkey=\nsock-port=2333\nsock-"
        "que-size=10\n\n# "
        "mysql\nip=127.0.0.1\nusername=root\npassword=\ndbname=userinfo\ndb-"
        "port="
        "3306\ndb-que-size=5\n\n# 版本号（请不要更改此项）\nversion=v0.0.1-pre";
    ifstream ifile;
    ofstream ofile;
    ifile.open("config.ini", ios::in);
    if (ifile.fail()) {
        ofile.open("config.ini", ios::out);
        ofile << DefaultConfig;
    }
    ifile.close();
    ofile.close();
    string cert, _key;
    ifstream file;
    string line;
    file.open("config.ini", ios::in);
    if (file.fail()) {
        nLog.Output("读取配置文件失败！", level::Fatal);
        nLog.Close();
        exit(0);
    }
    while (getline(file, line)) {
        int Idx = line.find('=');
        if (Idx == -1)
            continue;
        int EndIdx = line.find('\n', Idx);
        string key = line.substr(0, Idx);
        string value = line.substr(Idx + 1, EndIdx - Idx - 1);
        if (key == "cert")
            cert = value;
        else if (key == "key")
            _key = value;
    }
    file.close();
    // check key
    if (cert.empty() || _key.empty()) {
        cert = "cacert.pem";
        _key = "privkey.pem";
    }
    // init key&crt
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    EVP_PKEY_CTX *ctx = nullptr;
    EVP_PKEY *pkey = nullptr;
    BLOCK(keygen) {
        FILE *PublicKey = nullptr;
        FILE *PrivateKey = nullptr;
        PublicKey = fopen(cert.c_str(), "r");
        PrivateKey = fopen(_key.c_str(), "r");
        if (PublicKey && PrivateKey) {
            fclose(PublicKey);
            fclose(PrivateKey);
            break;
        }
        if (PublicKey || PrivateKey) {
            fclose(PublicKey ? PublicKey : PrivateKey);
            nLog.Output("密钥文件不完整，正在重新生成...", level::Warn);
        }
        ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        const EVP_CIPHER *cipher = EVP_aes_256_cfb128();
        try {
            if (!ctx)
                throw GetErrBuf();
            if (EVP_PKEY_keygen_init(ctx) == 0)
                throw GetErrBuf();
            if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 3072) <= 0)
                throw GetErrBuf();
            if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
                throw GetErrBuf();
        } catch (string buf) {
            nLog.Output(buf, level::Error);
            exit(0);
        }
        PublicKey = fopen(cert.c_str(), "w");
        PrivateKey = fopen(_key.c_str(), "w");
        if (!PEM_write_PUBKEY(PublicKey, pkey)) {
            nLog.Output(GetErrBuf(), level::Error);
            nLog.Close();
            exit(0);
        }
        if (!PEM_write_PrivateKey(PrivateKey, pkey, cipher, nullptr, 0, nullptr,
                                  nullptr)) {
            nLog.Output(GetErrBuf(), level::Error);
            nLog.Close();
            exit(0);
        }
        fclose(PublicKey);
        fclose(PrivateKey);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
    }
    ENDBLOCK(keygen);
    // Key generation code block end
    db = MysqlPool(&nLog);
    sock = MySocket(&nLog, &db);
}

int Service::Run(int type) {
    sock.Run(type);
    return 0;
}
