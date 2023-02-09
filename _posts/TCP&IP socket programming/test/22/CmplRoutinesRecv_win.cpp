// CmplRoutineRecv_win.cpp

#include <iostream>
#include <winsock2.h>

using namespace std;

#define BUF_SIZE 1024
void CALLBACK	CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void			ErrorHandling(char *msg);

WSABUF	dataBuf;
char	buf[BUF_SIZE];
int		recvBytes = 0;

int main(int argc, char *argv[])
{
	WSADATA			wsaData;
	SOCKET			hLisnSock, hRecvSock;
	SOCKADDR_IN		lisnAdr, recvAdr;

	WSAEVENT		evObj;
	WSAOVERLAPPED	overlapped;
	int				recvAdrSz, flags = 0;

	if(argc != 2)
	{
		cout << "Usage : " << argv[0] << " <port>\n";
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family = AF_INET;
	lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAdr.sin_port = htons(atoi(argv[1]));

	if (::bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (::listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAdrSz = sizeof(recvAdr);
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);
	if (hRecvSock == INVALID_SOCKET)
		Errorhandling("accept() error");

	evObj = WSACreateEvent();	// Dummy event object
	memset(&overlapped, 0, sizeof(overlapped));
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = msg;

	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSA_IO_PENDING)
			cout << "Background data receive\n";
	}

	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE);
	if (idx == WAIT_IO_COMPLETION)
		cout << "Overlapped I/O Completed\n";
	else	// if error occurred!
		ErrorHandling("WSARecv() error");

	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK	CompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVErLAPPED lpOverlapped, DWORD flags)
{
	if (dwError != 0)
		ErrorHandling("CompRoutine error");
	else
	{
		recvBytes = szRecvBytes;
		cout << "Received message : " << buf << endl; 
	}
}

void	ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}