#include "service.h"

#include <fstream>

using std::ifstream;
using std::ofstream;
using std::ios;

Service::Service(){
	DefaultConfig="# socket\ncert=cacert.pem\nkey=privkey.pem\nsock-port=2333\nsock-que-size=10\n\n# mysql\nip=127.0.0.1\nusername=root\npassword=\ndbname=userinfo\ndb-port=3306\ndb-que-size=5";
	ifstream ifile;
	ofstream ofile;
	ifile.open("config.ini", ios::in);
	if(ifile.fail()){
		ofile.open("config.ini", ios::out);
		ofile<<DefaultConfig;
	}
	nLog=Logger(5);
	db=MysqlPool(&nLog);
	sock=MySocket(&nLog, &db);
}

int Service::Run(int type){
	sock.Run(type); 
	return 0;
}
