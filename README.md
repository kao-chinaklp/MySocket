# 关于这个项目
就如简介所说，这是个用来练手的socket项目

## 构成
主体：MySocket  
更新器：MySocket-updater  
[传送门]("https://github.com/kao-chinaklp/MySocket-Updater")

## 关于本体
本体调用了curl、openssl和mysql的库，更新器调用了curl和json的库，但库本体不保留在项目中，编译/运行的时候记得自行添加。

## 更新日志
### 2023 6.24  
完工！（我以为的）  
### 2023 7.10  
更新了cmake的编译配置文件  
### 2023 7.11  
添加了不完整自动生成证书文件的功能  
### 2023 7.12  
没想到会有这么多bug要修（悲  
1.优化了默认配置文件的写入内容；  
2.日志文件不再生成在与可执行文件同一目录下而是统一放在logs文件夹；  
3.修复了log系统不能正常关闭的问题；  
4.修复了MySocket中正则表达式不能正常发挥作用的问题；  
5.修复了一个读取配置文件时的小bug；  
6.更改了部分提示的内容；  
7.修复了mysqlpool中正则表达式不能正常发挥作用的问题；  
8.将SSL的加密方式改为更安全的TLS;  
9.修改了CMakeLists.txt中导致不能正常生成可执行文件的内容。  
### 2023 7.13
1.遇到错误时不是直接退出程序而是采用异常处理。（感谢moyongxin的帮助）；  
2.将所有模块的关闭函数放在析构函数中，使其能自动关闭；  
3.完善了mysocket中socket在不同平台下的相应的代码；  
3.统一了所有模块的关闭函数名（Close）；  
4.将mysocket构造函数中冗长的代码移到了Init函数中；  
5.修改了部分注释；  
6.解决了重复释放空间导致的段错误；  
7.在Service模块中所有子模块不再是成员变量而是指针；  
8.添加对线程池的线程状态状态修改的功能，并解决了原线程池中有线程没有完成任务就被释放的问题。  
9.把默认配置文件单独放在一个文件里面；  
10.修复了证书无法正常生成的bug（但生成的证书无法被SSL识别并使用）。  
### 2023 7.14
1.修复了了Windows下socket销毁时可能会产生的段错误；  
2.修复了使用证书文件时，没有加载密码的问题；  
3.Windows套接字不能正常释放空间；  
4.更安全地释放所有模块的空间；  
5.修复了线程池退出时段错误的问题；  
6.默认配置文件的排版问题；  
7.MySQLPool中线程池的初始化没有传参；  
8.意外的代码顺序导致线程池死锁;  
9.将静态文本全部移至context.h。
### 2023 7.22
1.修复了配置文件中server-ip无法使用默认值的问题；  
2.优化了socket读取配置文件时合法性的判断。
### 2023 7.23
1.正式修复证书无法正确生成的功能；  
2.将证书生成的功能相关代码移至单独的函数；  
3.添加了Readme的使用前必读文件；  
4.添加了生成证书时手动输入持有者信息的功能；  
5.修复了一处文本错误。
### 2023 7.27
1.将版本号独立出来（context.h）；  
2.日志系统添加了Input选项，修改了一处文本错误；  
3.日志系统添加了部分日志的色彩；   
4.完成了自动更新的功能。
### 2023 7.28
1.完成了数据库操作的相关功能；  
2.添加了账号注销的功能；  
3.会自动识别校验数据表的正确性；  
4.更新了配置文件的格式；  
5.删除了mysocket中一处多余的文本；  
6.添加了一处注释(service.cpp)。

## 待修复bug列表 
暂无 

## 待添加的功能（最好能添加）
1.服务端可用的指令功能；  
2.文件传输功能。
