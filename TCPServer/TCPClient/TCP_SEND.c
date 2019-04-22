#include <windows.h>
#include <stdio.h>
#include <winsock.h>

char DataBuffer[1024];

int main(int argc, char** argv) {

	struct sockaddr_in SockAddrServer;
	SOCKET SockData = INVALID_SOCKET;
	struct hostent* pHostEnt;
	int nPortServer, nMsgLen, i;
	WSADATA WSAData;
	WORD wWSAVer;

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//командная строка
	if (argc < 3) {
		puts("Not enough arguments\n");
		puts("Вызов: TCP_SEND <addr/name> <port> <msg1> ...\n");
		return -1;
	}
	//инициализация подсистемы сокетов
	wWSAVer = MAKEWORD(1, 1);
	if (WSAStartup(wWSAVer, &WSAData) != 0) {
		puts("Error initializing WinSocket");
		return -1;
	}
	//подготовка адреса сервера
	memset(&SockAddrServer, 0, sizeof(SockAddrServer));
	SockAddrServer.sin_family = AF_INET;
	if (strcmp(argv[1], "255.255.255.255") == 0) //адрес broadcast
		SockAddrServer.sin_addr.S_un.S_addr = INADDR_BROADCAST;
	else {
		SockAddrServer.sin_addr.S_un.S_addr = inet_addr(argv[1]);
		if (SockAddrServer.sin_addr.S_un.S_addr == INADDR_NONE) {
			if ((pHostEnt = gethostbyname(argv[1])) == NULL) {
				fprintf(stderr, "Host not defined: %s\n", argv[1]);
				return -1;
			}
			SockAddrServer.sin_addr = *(struct in_addr*)(pHostEnt->h_addr_list[0]);
		}
	}
	if (sscanf_s(argv[2], "%u", &nPortServer) < 1) {
		fprintf(stderr, "Wrong port number: %s\n", argv[2]);
		return -1;
	}
	SockAddrServer.sin_port = htons((unsigned short)nPortServer);
	//создание локального сокета
	SockData = socket(PF_INET, SOCK_STREAM, 0);
	if (SockData == INVALID_SOCKET) {
		fputs("Creating socket error\n", stderr);
		return -1;
	}
	//запрос на установление соединения
	if (connect(SockData,
		(const struct sockaddr*)&SockAddrServer, sizeof(SockAddrServer)) != 0)
	{
		fprintf(stderr, "Connection error with %s:%u\n",
			inet_ntoa(SockAddrServer.sin_addr),
			ntohs(SockAddrServer.sin_port)
		);
		closesocket(SockData);
		return -1;
	}
	fprintf(stdout, "Success connection with: %s:%u \n",
		inet_ntoa(SockAddrServer.sin_addr),
		ntohs(SockAddrServer.sin_port)
	);
	//рабочий цикл
	for (i = 3; i < argc; ++i) { //остальные аргументы командной строки
	//отослать сообщение
		fprintf(stdout, "Sending: \"%s\" \n", argv[i]);
		nMsgLen = sizeof(DataBuffer) - 1;
		if (send(SockData, argv[i], nMsgLen, 0) < nMsgLen) {
			fprintf(stderr, "Sending error: \"%s\"\n", argv[i]);
			continue;
		}
		//принять и отобразить ответ
		nMsgLen = recv(SockData, DataBuffer, sizeof(DataBuffer) - 1, 0);
		if (nMsgLen <= 0) { //ошибка приема ответа
			fputs("Getting message error \n", stderr);
			continue;
		}
		DataBuffer[nMsgLen] = '\0';
		fprintf(stdout, "Answer from %s:%u: %s \n",
			inet_ntoa(SockAddrServer.sin_addr),
			ntohs(SockAddrServer.sin_port),
			DataBuffer
		);
	}
	//завершение
	shutdown(SockData, 2);
	Sleep(100);
	closesocket(SockData); SockData = INVALID_SOCKET;
	WSACleanup();
	return 0;
}