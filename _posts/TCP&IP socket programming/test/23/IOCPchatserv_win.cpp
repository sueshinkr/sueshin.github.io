// IOCPchatserv_win.cpp
// 클라이언트는 이전의 chat_clnt_win.cpp를 사용

#include <iostream>
#include <process.h>
#include <windows.h>
#include <winsock2.h>

using namespace std;

#define BUF_SIZE	100
#define MAX_CLNT	256

typedef struct	// socket info
{
	SOCKET		hClntSock;
	SOCKADDR_IN	clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct	// buffer info
{
	OVERLAPPED	overlapped;
	WSABUF		wsaBuf;
	char		buffer[BUF_SIZE];
} PER_IO_DATA, *LPPER_IO_DATA;

DWORD WINAPI	EchoThreadMain(LPVOID CompletionPortIO);
void			ErrorHandling(char *message);

int		clntCnt = 0;
SOCKET	clntSocks[MAX_CLNT];

int main(int argc, char *argv[])
{
	WSADATA				wsaData;
	HANDLE				hComPort;
	SYSTEM_INFO			sysInfo;
	LPPER_IO_DATA		ioInfo;
	LPPER_HANDLE_DATA	handleInfo;

	SOCKET			hServSock;
	SOCKADDR_IN		servAdr;
	int				recvBytes, flags = 0;


	if(argc != 2)
	{
		cout << "Usage : " << argv[0] << " <port>\n";
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);
	for (int i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);
	
	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (::bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (::listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1)
	{
		SOCKET		hClntSock;
		SOCKADDR_IN	clntAdr;
		int			addrLen = sizeof(clntAdr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		clntSocks[clntCnt++] = hClntSock;

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

DWORD WINAPI	EchoThreadMain(LPVOID pComPort)
{
	HANDLE				hComPort = (HANDLE)pComPort;
	SOCkET				sock;
	DWORD				bytesTrans;
	LPPER_HANDLE_DATA	handleInfo;
	LPPER_IO_DATA		ioInfo;
	DWORD				flags = 0;

	while (1)
	{
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = handleInfo->hClntSock;

		cout << "message received!\n";
		if (bytestrans == 0)	// EOF 전송시
		{
			for (int i = 0; i < clntCnt; i++)
			{
				if (sock == clntSocks[i])
				{
					while (i++ < clntCnt - 1)
						clntSocks[i] = clntSocks[i + 1];
				}
			}
			clntCnt--;
			closesocket(sock);
			free(handleInfo);
			free(ioInfo);
			continue;
		}

		for (int i = 0; i < clntCnt; i++)
			send(clntSocks[i], ioInfo->buffer, bytesTrans, 0);

		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

void	ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
