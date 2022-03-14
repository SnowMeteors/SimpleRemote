### Simple Remote
一个用c/c++写的基于tcp通信的简单远控，写此项目主要是将自己所学的知识融会贯通，仅此而已。

代码健壮性不是非常的高，在输入一些非法的命令格式的时候，有可能会引起崩溃，我也懒得处理了。毕竟这不是我写这个项目的重点。

视频演示: <a href="https://www.bilibili.com/video/BV1aq4y1e7nU">BV1aq4y1e7nU</a>

### 建立连接

本项目分为客户端（Client.exe） 和服务器端（Server.exe）。因为使用的是反向连接方式，所以必须先启动Server.exe

启动Server.exe，格式 `Server.exe port`。示例`Server.exe 3333`，将会监听`3333`端口

启动Client.exe，格式  `Client.exe ip port`。示例`Client.exe 192.168.0.1 3333`。将会连接192.168.0.1的3333端口

 <img src="image\3.jpg">


### 支持功能
#### help
查看支持命令, 格式 `help`
<img src="image\11.jpg">

#### upload
上传本地文件到对方机器。格式  `upload filepath`。
<img src="image\1.jpg">

#### download

下载对方机器文件到本地。格式  `download filepath`。
<img src="image\2.jpg">

#### ps
列出正在运行的进程，格式 `ps`
<img src="image\4.jpg">

#### getpid
获取当前进程pid，格式 `getpid`
<img src="image\5.jpg">

#### drive
列出盘符，格式 `drive`
<img src="image\6.jpg">

#### screen
屏幕截图，格式 `screen`
<img src="image\7.jpg">
#### nc
反弹nc，格式 `nc ip port`
<img src="image\8.jpg">
#### msf
反弹msf，仅支持`windows/meterpreter/reverse_tcp`，格式 `msf ip port`
<img src="image\9.jpg">
#### getsystem
从管理员提权到system，格式 `getsystem`，如果提权成功会打开一个新窗口
<img src="image\10.jpg">

### 待完善
1.反弹msf功能，因为在client.exe启动了一个msf的子线程，所以msf退出或者client.exe退出。都会导致对方程序崩溃，后续会考虑使用傀儡进程
2.流量层进行加密
3.添加bypassuac功能
4.添加端口转发功能
5.添加内网存活主机扫描功能
6.先阶段只能连接一个客户端，后续考虑支持多个客户端 （最后考虑）
7.支持交互式shell （最后考虑）
