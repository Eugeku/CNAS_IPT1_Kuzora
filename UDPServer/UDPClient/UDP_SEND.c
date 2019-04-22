#include <windows.h>
#include <stdio.h>
#include <winsock.h>

char DataBuffer[1024];
int main(int argc, char** argv) {
	struct sockaddr_in SockAddrLocal, SockAddrSend, SockAddrRecv;
	SOCKET SockLocal = INVALID_SOCKET;
	struct hostent* pHostEnt;
	int nSockOptBC, nAddrSize, nPortRemote, nMsgLen, i;
	WSADATA WSAData;
	WORD wWSAVer;

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	//командная строка
	if (argc < 3) {
		puts("Not enougn arguments\n");
		puts("Calling: UDP_SEND <addr/name> <port> <msg1> ...\n");
		return -1;
	}
	//инициализация подсистемы сокетов
	wWSAVer = MAKEWORD(1, 1);
	if (WSAStartup(wWSAVer, &WSAData) != 0) {
		puts("Initialisation error WinSockets");
		return -1;
	}
	//создание локального сокета
	SockLocal = socket(PF_INET, SOCK_DGRAM, 0);
	if (SockLocal == INVALID_SOCKET) {
		fputs("Socket creation error\n", stderr);
		return -1;
	}
	//настройка сокета: разрешить отсылку на "широковещательный" адрес
	nSockOptBC = 1;
	setsockopt(SockLocal,
		SOL_SOCKET, SO_BROADCAST,
		(char*)(&nSockOptBC), sizeof(nSockOptBC)
	);
	//привязка сокета к "локальному" адресу
	memset(&SockAddrLocal, 0, sizeof(SockAddrLocal));
	SockAddrLocal.sin_family = AF_INET;
	SockAddrLocal.sin_addr.S_un.S_addr = INADDR_ANY;
	SockAddrLocal.sin_port = 0;
	if (bind(SockLocal, (struct sockaddr*)&SockAddrLocal, sizeof(SockAddrLocal)) != 0)
	{
		fputs("Error open localhost socket\n", stderr);
		return -1;
	}
	//подготовка адреса сервера
	memset(&SockAddrSend, 0, sizeof(SockAddrSend));
	SockAddrSend.sin_family = AF_INET;
	if (strcmp(argv[1], "255.255.255.255") == 0)
		SockAddrSend.sin_addr.S_un.S_addr = INADDR_BROADCAST;
	else {
		SockAddrSend.sin_addr.S_un.S_addr = inet_addr(argv[1]);
		if (SockAddrSend.sin_addr.S_un.S_addr == INADDR_NONE) {
			if ((pHostEnt = gethostbyname(argv[1])) == NULL) {
				fprintf(stderr, "Host not defined: %s\n", argv[1]);
				return -1;
			}
			SockAddrSend.sin_addr = *(struct in_addr*)(pHostEnt->h_addr_list[0]);
		}
	}
	if (sscanf_s(argv[2], "%u", &nPortRemote) < 1) {
		fprintf(stderr, "Wrong port number: %s\n", argv[2]);
		return -1;
	}
	SockAddrSend.sin_port = htons((unsigned short)nPortRemote);
	//рабочий цикл
	for (i = 3; i < argc; ++i) { //остальные аргументы командной строки
	//отослать сообщение
		fprintf(stdout, "Send on %s:%u: \"%s\" \n",
			inet_ntoa(SockAddrSend.sin_addr),
			ntohs(SockAddrSend.sin_port),
			argv[i]
		);
		nMsgLen = sizeof(DataBuffer)-1;

		if (sendto(SockLocal, argv[i], nMsgLen, 0,
			(struct sockaddr*) &SockAddrSend, sizeof(SockAddrSend)) < nMsgLen)
		{
			fprintf(stderr, "Sending issue: \"%s\"\n", argv[i]);
			continue;
		}
		//принять и отобразить ответ  sizeof(DataBuffer)
		nAddrSize = sizeof(SockAddrRecv);
		nMsgLen = recvfrom(SockLocal, DataBuffer, sizeof(DataBuffer) - 1, 0,
			(struct sockaddr*) &SockAddrRecv, &nAddrSize
		);
		if (nMsgLen <= 0) { //ошибка приема запроса
			fputs("Getting message error\n", stderr);
			continue;
		}
		DataBuffer[nMsgLen] = '\0';
		fprintf(stdout, "Answer from %s:%u: %s \n", 
			inet_ntoa(SockAddrRecv.sin_addr), 
			ntohs(SockAddrRecv.sin_port),
			DataBuffer
		);
	}
	//завершение
	shutdown(SockLocal, 2);
	Sleep(100);
	closesocket(SockLocal); 
	SockLocal = INVALID_SOCKET;
	WSACleanup();
	return 0;
}