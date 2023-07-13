#include "service.h"
#include "util.h"

int main() {
    system("chcp 65001");
    Service *Ser = nullptr;
    try {
        Ser = new Service();
        Ser->Init();
        Ser->Run(1);
    } catch (const safe_exit e) {
        if (Ser != nullptr) {
            delete Ser;
        }
        return e.code;
    };
    return 0;
}