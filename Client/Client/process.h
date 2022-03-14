#pragma once
#include "command.h"
#include <sddl.h>
#include <WtsApi32.h>

#pragma comment(lib,"Wtsapi32.lib")

/**
 *
 * 进程相关的操作
 * 麦当
 * 2022/3/9
 *
**/

// 启用调试权限
void EnableDebugAbility();
// 获取域名\用户名
void GetAccount(PSID userSid,char buffer[]);
// 获取进程列表指针
PWTS_PROCESS_INFO_EX GetProcessList(DWORD* processCnt);
// 获取指定PID进程的位数 0:获取失败 32:32位程序 64:64位程序
int GetProcessPlatform(DWORD dwProcessID);
// 查看进程信息命令
BOOL PS(SOCKET sServer, char cmd[]);
// 父进程欺骗
BOOL PPIDSpoofing(SOCKET sServer,DWORD pid);