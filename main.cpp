#include "service.h"

int main(){
    Service* Ser=nullptr;
    try{
        Ser=new Service;
        Ser->Init();
        Ser->Run(type::tcp);
    }
    catch(const int code){
        if(Ser!=nullptr)delete Ser;
        return code;
    }
    return 0;
}