#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define BUF_SIZE 100
#define READ	3
#define	WRITE	5
#define MAX_Client 3

#pragma comment(lib,"ws2_32.lib")

typedef struct    // socket info
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct    // buffer info
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;    // READ or WRITE
} PER_IO_DATA, *LPPER_IO_DATA;

int serv_num[3];
int clnt_num[3]; 
int score[2]; 

char toStrike[100];
char toBall[100];
char toClnt[100];

char temp2[100] = "그냥주는버퍼";
char *recvBuf=NULL;



DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char *message);
void CompressSockets(LPPER_HANDLE_DATA hSockArr[], SOCKET soc);

HANDLE hMutex;

unsigned threadId;

int main(int argc, char* argv[])
{
	WSADATA	wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
	SOCKADDR_IN servAdr;

	int recvBytes, i, flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);

	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, &threadId);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(50000);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);
	printf("클라이언트의 접속을 기다립니다......\n");

	srand((unsigned)time(NULL));

	while ((serv_num[0] == serv_num[1]) |
		(serv_num[1] == serv_num[2]) |
		(serv_num[2] == serv_num[0]))
	{
		serv_num[0] = rand() % 10;
		serv_num[1] = rand() % 10;
		serv_num[2] = rand() % 10;
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
	printf("서버 숫자 ---> %d  %d  %d\n", serv_num[0], serv_num[1], serv_num[2]);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

	while (1)
	{
		int addrLen = sizeof(clntAdr);
		
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);
		printf("클라이언트가 접속\n");
		printf("숫자 야구 게임 시작...!!!!\n");

		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf),
			1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

DWORD WINAPI EchoThreadMain(LPVOID pComPort)
{
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	
	int strike = 0, ball = 0;


	while (1)
	{
		
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
			(LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = handleInfo->hClntSock;

		memset(score, 0, sizeof(score));
		memset(toClnt, 0, sizeof(toClnt));
		memset(toBall, 0, sizeof(toBall));
	

		if (ioInfo->rwMode == READ)
		{

			if (bytesTrans == 0)
			{
				printf("클라이언트 종료\n");
				closesocket(sock);
				free(handleInfo); free(ioInfo);
				continue;
			}

			recvBuf = ioInfo->buffer;

			for (int i = 0; i < 3; i++)
				 clnt_num[i] = recvBuf[i] - 48;
			
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
			printf("클라이언트 ---> %d  %d  %d\n", clnt_num[0],clnt_num[1],clnt_num[2]);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			for (int i = 0; i < 3; i++)
			{
				int oneserv_num = serv_num[i];
				for (int j = 0; j < 3; j++) {
					int oneClntNum = clnt_num[j];
					if (oneserv_num == oneClntNum) {
						if (i == j)
							score[0]++;
						else
							score[1]++;
					}
				}
			}
			int strike = score[0];
			int ball = score[1]; 
			
			sprintf(toClnt, "%d", strike);
			strcat(toClnt, "S");
			sprintf(toBall, "%d", ball);
			strcat(toBall, "B");
			strcat(toClnt, toBall);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
			printf("          %s\n", toClnt);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			
			
		
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = (long)100;
			ioInfo->wsaBuf.buf = toClnt; //보내는 버퍼
			ioInfo->rwMode = WRITE;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);



			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else
		{
			free(ioInfo);
		}
		
	}
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}