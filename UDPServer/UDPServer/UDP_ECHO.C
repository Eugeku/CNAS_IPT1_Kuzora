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

char DataBuffer[1024], *end;

int main(int argc, char** argv) {
	struct sockaddr_in SockAddrLocal, SockAddrRemote;
	SOCKET SockLocal = INVALID_SOCKET;
	unsigned short nPort = DEFAULT_ECHO_PORT;
	int nAddrSize, nCnt;
	WSADATA WSAData;
	WORD wWSAVer;

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	//разбор командной строки: номер порта
	if (argc > 1)
		if (sscanf_s(argv[1], "%hd", &nPort) < 1)
			fprintf(stderr, "Wrong port: %d, use default", nPort);
	//инициализация подсистемы сокетов
	wWSAVer = MAKEWORD(1, 1);
	if (WSAStartup(wWSAVer, &WSAData) != 0) {
		puts("Init error WinSocket");
		return -1;
	}
	//создание локального сокета
	SockLocal = socket(PF_INET, SOCK_DGRAM, 0);
	if (SockLocal == INVALID_SOCKET) {
		fputs("Creating error socket\n", stderr);
		return -1;
	}
	//привязка сокета к локальному адресу
	memset(&SockAddrLocal, 0, sizeof(SockAddrLocal));
	SockAddrLocal.sin_family = AF_INET;
	SockAddrLocal.sin_addr.S_un.S_addr = INADDR_ANY;
	SockAddrLocal.sin_port = htons(nPort); //(<номер_порта_сервера>);
	if (bind(SockLocal, (struct sockaddr*) &SockAddrLocal, sizeof(SockAddrLocal)) != 0) {
		fprintf(stdout, "Error, port %u\n",
			ntohs(SockAddrLocal.sin_port)
		);
		return -1;
	}
	fprintf(stderr, "Server run, port %u\n",
		ntohs(SockAddrLocal.sin_port)
	);
	//основной рабочий цикл
	while (1) { //для сервера цикл обычно бесконечен
		nAddrSize = sizeof(SockAddrRemote);
		//принять входящее сообщение ("эхо-запрос")
		nCnt = recvfrom(SockLocal,
			DataBuffer, sizeof(DataBuffer) - 1, 0,
			(struct sockaddr*) &SockAddrRemote, &nAddrSize
		);
		if (nCnt < 0) { //ошибка приема запроса
			fputs("Getting message error\n", stderr);
			continue;
		}

		int* array = parseCharsToInt(DataBuffer);

		if (isRightFormatForDate(array[0], array[1], array[2]) && isRightFormatForDate(array[3], array[4], array[5])) {
			int dateDiff = getDateDifference(array[0], array[1], array[2], array[3], array[4], array[5]);
			fprintf(stdout, "Results on server: %d \n", dateDiff);
			sprintf(DataBuffer, "Days different: %d \n", dateDiff);
		} else {
			char errorOrder[] = " are wrong dates";
			strcat(DataBuffer, errorOrder);
		}

		fprintf(stdout, "Get and send message: %s \n", DataBuffer);
		sendto(SockLocal, DataBuffer, nCnt, 0, (struct sockaddr*) &SockAddrRemote, sizeof(SockAddrRemote));
	}
	shutdown(SockLocal, 2);
	Sleep(100);
	closesocket(SockLocal); SockLocal = INVALID_SOCKET;
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
