#include "socket.h"

// 初始化sock
void InitSock()
{
	WSADATA wsa;
	int ret;
	// 初始化socket 使用2.2版本 成功返回0
	ret = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (ret)
	{
		printf("WSAStartup fail\n");
		exit(-1);
	}
}

// 建立连接
SOCKET Connect(char* ip, int port)
{
	int ret;

	SOCKET sServer = socket(AF_INET, SOCK_STREAM, 0);
	if (sServer == INVALID_SOCKET)
	{
		printf("socket fail\n");
		exit(-1);
	}

	// 设置发送地址
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.S_un.S_addr = inet_addr(ip);
	printf("          =[ connect to %-15s%-5d                    ]\n\n", inet_ntoa(server.sin_addr), htons(server.sin_port));

	// 建立连接
	ret = connect(sServer, (SOCKADDR*)&server, sizeof(SOCKADDR));
	if (ret == SOCKET_ERROR)
	{
		printf("connect fail\n");
		exit(-1);
	}
	return sServer;
}

// 发送数据
void SendData(SOCKET sServer, char buffer[], int size)
{
	// 防止对方还没打开接收通道就发数据
	Sleep(500);

	int ret = send(sServer, buffer, size, 0);
	if (ret == SOCKET_ERROR || ret == 0)
	{
		printf("send fail\n");
		exit(-1);
	}
}

// 接收数据
void RecvData(SOCKET sServer, char* buffer, int size)
{
	int ret = recv(sServer, buffer, size, 0);
	if (ret == 0 || ret == SOCKET_ERROR)
	{
		printf("recv fail\n");
		exit(-1);
	}
}

