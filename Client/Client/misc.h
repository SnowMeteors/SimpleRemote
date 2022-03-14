#pragma once
#include "socket.h"

/**
 *
 * 定义杂七杂八的函数
 * 麦当
 * 2022/3/11
 *
**/

// 发送失败消息
void SendError(SOCKET sServer);
// 发送成功信息
void SendSuccess(SOCKET sServer);
// 是否接收到错误信息
BOOL IsRecvError(SOCKET sServer);
// INT转CHAR *
void IntToStr(int num, char* str);
// TCHAR转CHAR *
void TcharToStr(const TCHAR* tchar, char* _char);
// CHAR *转LPCWSTR
void StrToLpcwstr(char* str, LPCWSTR lpcwstr);
// LPWSTR转CHAR *
void LpwstrToStr(LPWSTR lpwstr, char* str);
// 从右往左查找特定字符
int Left(char buffer[], char chr);
// 从左往右查找特定字符
int Right(char buffer[], char chr);
// 生成a-b的随机数
int randomNumber(int a, int b);
// 提取ip地址
void GetIP(char src[], char dest[], int index, int size);
// 提取端口
void GetPort(char src[], char dest[], int index, int size);
// 检查ip和端口是否按格式输入
BOOL IsErrorFormat(char cmd[], int index,int size);

