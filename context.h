//Global

#define ConfigReadingErr "读取配置文件失败！"
#define CheckCorrectnessF "请检查 "
#define CheckCorrectnessB " 项是否填写正确！"

// Service

#define LoggerStart "日志系统启动成功！"
#define DefaultCfg "# socket（密钥文件留空表示自动生成，key-psw项留空表示使用默认密码123456，server-ip留空表示使用127.0.0.1）\n\
cert=\nkey=\nkey-psw=\nserver-ip=\nsock-port=2333\nsock-que-size=10\n\n\
# mysql\nip=47.240.92.80\nusername=userinfo\npassword=mima123.\n\
dbname=userinfo\ndb-port=3306\ndb-que-size=5\n\n\
# 版本号（请不要更改此项）\nversion=v0.0.1-pre\n"
#define KeyfileIncomplete "密钥文件不完整，正在重新生成..."
#define KeyfileWSuccess "证书成功生成！"

// Account

#define AccountErr "未知错误！"
#define LoginErr "账号或密码错误！"
#define RegisterErr "账号已经被注册！"

// Logger

#define LogFatal "无法打开日志文件！"
#define LogClose "日志系统正在关闭..."

// MySocket

#define ConnectFailed "连接失败！"
#define ConnectionLost "链接失效！"
#define SSLLoadSuccess "SSL载入成功！"
#define CreateFailed "创建失败！"
#define VersionWrong "版本不符！"
#define ServiceClose "正在关闭服务..."
#define SocketClose "套接字已关闭。"
#define CreateSockFailed "创建套接字失败！"
#define SuccessStartF "套接字启动成功！正在监听"
#define SuccessStartB "端口"
#define TryConnect "尝试连接..."
#define BindFatal "无法绑定指定IP和端口"
#define ListenFatal "设置监听失败！"
#define ConnectFatal " 连接失败！"

// MySQLPool

#define OperateErr "无法操作数据库，错误码："
#define DBConnectSuccess "数据库连接成功！"
#define DBConnectFatal "数据库连接失败！错误码："
#define DBDisconnect "数据库连接已断开。"