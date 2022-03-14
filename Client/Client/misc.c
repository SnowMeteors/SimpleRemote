#include "misc.h"

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
	if (strcmp(buffer, "error") == 0)
	{
		return TRUE;
	}
	return FALSE;
}

// INT转CHAR *
void IntToStr(int num, char* str)
{
	itoa(num, str, 10);
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

// LPWSTR转CHAR *
void LpwstrToStr(LPWSTR lpwstr, char* str)
{
	int num = WideCharToMultiByte(CP_OEMCP, (DWORD)NULL, lpwstr, -1, NULL, 0, NULL, FALSE);
	WideCharToMultiByte(CP_OEMCP, (DWORD)NULL, lpwstr, -1, str, num, NULL, FALSE);
}

// 生成a-b的随机数
int randomNumber(int a, int b)
{
	int random;
	random = rand() % (b - a + 1) + a;
	return random ;
}

// CHAR *转LPCWSTR
void StrToLpcwstr(char* str, LPCWSTR lpcwstr)
{
	memset(lpcwstr, 0, sizeof(lpcwstr));
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, lpcwstr, MAX_PATH);
}

// 从右往左查找特定字符
int Left(char buffer[], char chr)
{
	int index = -1;
	int len = strlen(buffer);
	for (int i = len - 1; i >= 0; i--)
	{
		if (buffer[i] == chr)
		{
			index = i;
			break;
		}
	}

	return index;
}

// 从左往右查找特定字符
int Right(char buffer[], char chr)
{
	int index = -1;
	int len = strlen(buffer);

	for (int i = 0; i < len; i++)
	{
		if (buffer[i] == chr)
		{
			index = i;
			break;
		}
	}
	return index;
}

// 提取ip地址
void GetIP(char src[], char dest[], int index,int size)
{
	strncpy(src, dest + size + 1, index - size -1);
}

// 提取端口
void GetPort(char src[], char dest[], int index,int size)
{
	strncpy(src, dest + index + 1, strlen(dest) - index);
}

// 检查nc或者msf命令是否按格式输入
BOOL IsErrorFormat(char cmd[], int index,int size)
{
	if (cmd[index - 1] == ' ' || index <= size)
	{
		return TRUE;
	}
	return FALSE;
}
