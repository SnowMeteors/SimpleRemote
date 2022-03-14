#pragma once
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


/**
 * 
 * socket相关的操作
 * 麦当
 * 2022/3/8
 * 
**/

// 初始化socket
void InitSock();
// 建立连接
SOCKET Connect(char* ip, int port);
// 发送数据
void SendData(SOCKET sServer, char buffer[], int size);
// 接收数据
void RecvData(SOCKET sServer, char* buffer, int size);