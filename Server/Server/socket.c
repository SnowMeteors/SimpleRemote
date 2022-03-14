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

// 监听端口
SOCKET Listen(int port)
{
	int ret;

	// 创建socket
	SOCKET sClient = socket(AF_INET, SOCK_STREAM, 0);
	if (sClient == INVALID_SOCKET)
	{
		printf("socket fail\n");
		exit(-1);
	}

	// 监听设置
	SOCKADDR_IN local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	// 接受任意ip数据
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 绑定端口
	ret = bind(sClient, (SOCKADDR*)&local, sizeof(SOCKADDR));
	if (ret == SOCKET_ERROR)
	{
		printf("bind fail\n");
		exit(-1);
	}

	// 开启监听
	ret = listen(sClient, 5);
	if (ret == SOCKET_ERROR)
	{
		printf("listen fail\n");
		exit(-1);
	}

	return sClient;
}

// 建立连接
SOCKET Connect(SOCKET sClient)
{
	// 与服务器建立连接
	SOCKADDR_IN server;
	int AddrSize = sizeof(SOCKADDR);
	SOCKET sServer = accept(sClient, (SOCKADDR*)&server, &AddrSize);
	if (sServer == INVALID_SOCKET)
	{
		printf("accept fail\n");
		return 0;
	}
	char ip[20];
	inet_ntop(AF_INET, (SOCKADDR_IN*)&server.sin_addr, ip, 16);
	printf("          =[ connect from %-15s%-5d                  ]\n\n", ip, ntohs(server.sin_port));
	//printf("connect from %s:%d\n\n", ip, ntohs(server.sin_port));

	return sServer;
}

// 发送数据
void SendData(SOCKET sServer, char buffer[], int size)
{
	// 防止对方还没打开接收通道就发数据
	Sleep(10);

	int ret;
	ret = send(sServer, buffer, size, 0);
	if (ret == 0 || ret == SOCKET_ERROR)
	{
		printf("send fail");
		exit(-1);
	}
}

// 接收数据
void RecvData(SOCKET sServer, char buffer[], int size)
{
	int ret = recv(sServer, buffer, size, 0);
	if (ret == 0 || ret == SOCKET_ERROR)
	{
		printf("recv fail\n");
		exit(-1);
	}
}

// 发送错误消息
void SendError(SOCKET sServer)
{
	char buffer[] = "error";
	SendData(sServer, buffer, sizeof(buffer));
}

// 发送成功信息
void SendSuccess(SOCKET sServer)
{
	char buffer[] = "success";
	SendData(sServer, buffer, sizeof(buffer));
}

// 是否接收到错误信息
BOOL IsRecvError(SOCKET sServer)
{
	char buffer[20] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));
	RecvData(sServer, buffer, sizeof(buffer));
	if (strcmp(buffer,"error") == 0)
	{
		return TRUE;
	}
	return FALSE;
}