/*
 * 主体服务声明
 * 基于线程池、数据库连接池、日志系统及openssl和curl库
*/
#ifndef SERVICE_H_
#define SERVICE_H_

#include "mysocket.h"

class Service{
    public:
        Service();
        ~Service();
        void Init();
        int Run(type _type);
        bool CheckUpdate();
        bool CheckReadme();
        void GenerateCertificate(string cert, string _key);// 生成证书

    private:
        // 相关功能模块
        string Version;
        Logger* nLog=nullptr;
        MysqlPool* db=nullptr;
        MySocket* sock=nullptr;
};

#endif
