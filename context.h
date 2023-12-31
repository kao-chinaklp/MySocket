//Global

#define _Version "v0.0.4"
#define ReadmeReadingErr "读取readme文件失败！"
#define ConfigReadingErr "读取配置文件失败！"
#define CheckCorrectnessF "请检查 "
#define CheckCorrectnessB " 项是否填写正确！"
#define CheckIPCorrectness "请检查 IP 项是否填写正确！"

// Service

#define Readme "关于本项目\n\
本项目只是练手用的项目，仅供参考，不要投入生产环境中！\n\n\
关于证书生成\n\
生成的证书具有一年的有效期\n\
本项目生成的证书不具备权威性，若有需要可自行向有关机构申请\n\n\
关于配置文件\n\
密钥文件留空表示自动生成；\n\
key-psw项留空表示使用默认密码123456；\n\
server-ip留空表示使用本地地址；mode项需填ipv4或ipv6，区分大小写；\n\
database项填数据库名（*.db），留空表示使用默认值（项目名）;tablename留空表示使用默认名字(userinfo)\n\
不要更改版本号！\n\
我已仔细阅读=false"
#define ReadmeConfirm "请在仔细阅读同目录下readme.txt文件后将文件中“我已仔细阅读”的值设置为true再继续运行该程序。"
#define NewVersion "存在更新版本，是否现在更新？(Y/N)"
#define StartUpdaterFail "无法启动更新程序！"
#define InitCurlFail "初始化curl失败"
#define LoggerStart "日志系统启动成功！"
#define GetUpdateFail "无法获取更新！"
#define DefaultCfg "# socket\n\
# 密钥文件留空表示自动生成，key-psw项留空表示使用默认密码123456；\n\
# server-ip留空表示使用本地地址；mode项需填ipv4或ipv6，区分大小写；\n\
# database项填数据库名（*.db），留空表示使用默认值（项目名）;tablename留空表示使用默认名字(userinfo)\n\
cert=\nkey=\nkey-psw=\nserver-ip=\nsock-port=2333\nsock-que-size=10\nmode=ipv4\n\
database=\ntablename=\ndb-que-size=3\n\n\
# 版本号（请不要更改此项）\nversion="
#define KeyfileIncomplete "密钥文件不完整，正在重新生成..."
#define KeyfileWSuccess "证书成功生成！"
#define InputArea "请输入所在国家或地区：（如CN/US）"
#define InputOwner "请输入证书持有者："
#define IllegalInput "无效输入！\n"
#define SignedFail "证书签名失败！："

// Account

#define AccountErr "未知错误！"
#define AccountIsnExist "账号不存在！"
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
#define SocketSetFailed "端口复用设置失败！"
#define BindSuccess "套接字绑定成功！"
#define SuccessStartF "套接字启动成功！正在监听"
#define SuccessStartF_v6 "套接字启动成功！正在监听["
#define SuccessStartB "端口"
#define TryConnect "尝试连接..."
#define BindFatal "无法绑定指定IP和端口"
#define ListenFatal "设置监听失败！"
#define ConnectFatal " 连接失败！"
#define UserLogout "用户登出"
#define UserLogoff "用户注销"
#define UserExist "用户已经登录！"

// MySQLPool

#define OperateErr "无法操作数据库，错误码："
#define DBConnectSuccess "数据库连接成功！"
#define DBConnectFatal "数据库连接失败："
#define TableCreateFailed "表创建失败："
#define TableIllegal "检测到表不符合要求，是否立刻重建表？（Y/N）"
#define TableRebuildFailed "表重建失败！"
#define TableRebuildSuccess "表重建成功！"
#define DatabaseInitFinished "数据初始化成功！"
#define DBDisconnect "数据库连接已断开。"