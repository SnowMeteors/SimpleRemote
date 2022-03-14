#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

/**
 *
 * socket相关的操作
 * 麦当
 * 2022/3/8
 *
**/

// 初始化sock
void InitSock();
// 监听端口
SOCKET Listen(int port);
// 建立连接
SOCKET Connect(SOCKET sClient);
// 发送数据
void SendData(SOCKET sServer, char buffer[], int size);
// 接收数据
void RecvData(SOCKET sServer, char buffer[], int size);
// 发送失败消息
void SendError(SOCKET sServer);
// 发送成功信息
void SendSuccess(SOCKET sServer);
// 是否接收到错误信息
BOOL IsRecvError(SOCKET sServer);