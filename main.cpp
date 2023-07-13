#include "service.h"

int main(){
    #ifdef _WIN32
    system("chcp 65001");
    #endif
    Service* Ser=nullptr;
    try{
        Ser=new Service;
        Ser->Init();
        Ser->Run(1);
    }
    catch(const int code){
        if(Ser!=nullptr)delete Ser;
        return code;
    }
    return 0;
}