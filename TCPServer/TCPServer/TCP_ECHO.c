#include <windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEFAULT_ECHO_PORT 7
#define _CRT_SECURE_NO_WARNINGS

extern int* parseCharsToInt(char DataBuffer[]);
extern int getDateDifference(int firstDay, int firstMon, int firstYear, int secondDay, int secondMon, int secondYear);
extern boolean isRightFormatForDate(int firstDay, int firstMon, int firstYear);

char DataBuffer[1024];

int main(int argc, char** argv) {
	struct sockaddr_in SockAddrBase, SockAddrPeer;
	SOCKET SockBase = INVALID_SOCKET, SockData = INVALID_SOCKET;
	unsigned short nPort = DEFAULT_ECHO_PORT;
	int nAddrSize, nCnt;
	WSADATA WSAData;
	WORD wWSAVer;

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	//разбор командной строки: номер порта
	if (argc > 1)
		if (sscanf_s(argv[1], "%u", &nPort) < 1)
			fprintf(stderr, "Wrong port: %s, use default", nPort);
	//инициализация подсистемы сокетов
	wWSAVer = MAKEWORD(1, 1);
	if (WSAStartup(wWSAVer, &WSAData) != 0) {
		puts("Init error WinSocket");
		return -1;
	}
	//создание локального сокета
	SockBase = socket(PF_INET, SOCK_STREAM, 0);
	if (SockBase == INVALID_SOCKET) {
		fputs("Creating error socket\n", stderr);
		return -1;
	}
	//привязка базового сокета к локальному адресу
	memset(&SockAddrBase, 0, sizeof(SockAddrBase));
	SockAddrBase.sin_family = AF_INET;
	SockAddrBase.sin_addr.S_un.S_addr = INADDR_ANY;
	SockAddrBase.sin_port = htons(nPort); //(<номер_порта_сервера>);
	if (bind(SockBase,
		(struct sockaddr*) &SockAddrBase, sizeof(SockAddrBase)
	) != 0)
	{
		fprintf(stderr, "Ошибка привязки к локальному порту: %u\n",
			ntohs(SockAddrBase.sin_port)
		);
		return -1;
	}
	//включение режима "прослушивания"
	if (listen(SockBase, 2) != 0) { //очередь на 2 места
		closesocket(SockBase);
		fputs("Ошибка включения режима прослушивания\n", stderr);;
		return -1;
	}
	fprintf(stderr,
		"Server run, port %u\n",
		ntohs(SockAddrBase.sin_port)
	);
	//основной рабочий цикл - прием и обслуживание соединений
	while (1) { //для сервера цикл обычно бесконечен
		nAddrSize = sizeof(SockAddrPeer);
		SockData = accept(SockBase,
			(struct sockaddr*)&SockAddrPeer, &nAddrSize
		);
		if (SockData == INVALID_SOCKET) {
			fputs("Ошибка приема соединения\n", stderr);
			continue;
		}
		//цикл обслуживания одного соединения
		while (1) {



			nCnt = recv(SockData, DataBuffer, sizeof(DataBuffer) - 1, 0);
			if (nCnt <= 0)
				break;

			int* array = parseCharsToInt(DataBuffer);

			if (isRightFormatForDate(array[0], array[1], array[2]) && isRightFormatForDate(array[3], array[4], array[5])) {
				int dateDiff = getDateDifference(array[0], array[1], array[2], array[3], array[4], array[5]);
				fprintf(stdout, "Results on server: %d \n", dateDiff);
				sprintf(DataBuffer, "Days different: %d \n", dateDiff);
			}
			else {
				char errorOrder[] = " are wrong dates";
				strcat(DataBuffer, errorOrder);
			}

			fprintf(stdout, "Send message: %s \n", DataBuffer);
			send(SockData, DataBuffer, nCnt, 0); //возврат "эха"
		}
		shutdown(SockData, 2);
		closesocket(SockData); SockData = INVALID_SOCKET;
	}
	//завершение - здесь никогда не достигается!
	shutdown(SockBase, 2);
	Sleep(100);
	closesocket(SockBase); SockBase = INVALID_SOCKET;
	WSACleanup();
	return 0;
}

int* parseCharsToInt(char DataBuffer[]) {
	char *str = DataBuffer, *p = str;
	int array[256];
	int index = 0;
	while (*p) {
		if (isdigit(*p)) {
			long val = strtol(p, &p, 10);
			array[index] = val;
			index++;
		}
		else {
			p++;
		}
	}
	return array;
}

int getDateDifference(int firstDay, int firstMon, int firstYear, int secondDay, int secondMon, int secondYear) {
	time_t t1, t2;
	struct tm start_date;
	struct tm end_date;
	double days;
	int initYear = 1900;

	start_date.tm_hour = 0;
	start_date.tm_min = 0;
	start_date.tm_sec = 0;
	start_date.tm_mon = firstMon;
	start_date.tm_mday = firstDay;
	start_date.tm_year = firstYear - initYear;

	end_date.tm_hour = 0;
	end_date.tm_min = 0;
	end_date.tm_sec = 0;
	end_date.tm_mon = secondMon;
	end_date.tm_mday = secondDay;
	end_date.tm_year = secondYear - initYear;

	t1 = mktime(&start_date);
	t2 = mktime(&end_date);

	days = difftime(t2, t1) / 60 / 60 / 24;
	printf("%lf\n", days);
	return (int)days;
}

boolean isRightFormatForDate(int firstDay, int firstMon, int firstYear) {
	// day of the month - [1, 31] 
	// months since January - [1, 12]
	// years since 1900
	return ((firstDay >= 1 && firstDay <= 31) && (firstMon >= 1 && firstMon <= 12));
}