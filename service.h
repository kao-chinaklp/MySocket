#ifndef SERVICE_H_
#define SERVICE_H_

#include "mysocket.h"

class Service{
    public:
        Service();
        ~Service();
        void Init();
        int Run(int type);
        bool CheckUpdate();
        bool CheckReadme();
        void GenerateCertificate(string cert, string _key);

    private:
        string Version;
        Logger* nLog=nullptr;
        MysqlPool* db=nullptr;
        MySocket* sock=nullptr;
};

#endif
