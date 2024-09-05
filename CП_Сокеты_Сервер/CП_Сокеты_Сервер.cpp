#include <iostream>
#include <WinSock2.h>|
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
int main(void)
{
	//Ключевые константы
	const char IP_SERV[] = "127.0.0.1"; //IP-адрес локального сервера
	const int PORT_NUM = 1234; //Открыть порт рабочего сервера
	const short BUFF_SIZE = 1024; //Максимальный размер буфера для обмена информацией между сервером и клиентом
	//Ключевые переменные для всей программы
	int erStat; //Сохраняет статус ошибок сокета
	//IP в строковом формате преобразовать в числовой формат для функций сокета. Данные находятся в "ip_to_num"
	in_addr ip_to_num;
	erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);
	if (erStat <= 0) {
		cout << "Error in IP translation to special numeric format" << endl;
		return 1;
	}
	//Инициализация WinSock
	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (erStat != 0) {
		cout << "Error WinSock version initializaion #";
		cout << WSAGetLastError();
		return 1;
	}
	else
		cout << "WinSock is initialized" << endl;
	//Инициализация сокета сервера
	SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0);
	if (ServSock == INVALID_SOCKET) {
		cout << "Error initialization socket # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		return 1;
	}
	else
		cout << "Server socket is initialized" << endl;
	// Привязка сокета сервера
	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo)); // Инициализация структуры servInfo
	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(PORT_NUM);
	erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));
	if (erStat != 0)
	{
		cout << "Error Socket binding to server info. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		return 1;
	}
	else
		cout << "Socket binding to server information completed" << endl;
	// Начинаем слушать любых Клиентов
	erStat = listen(ServSock, SOMAXCONN);
	if (erStat != 0)
	{
		cout << "Can`t start to listen to. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		return 1;
	}
	else
		cout << "Listening" << endl;
	// Создание и прием клиентского сокета при подключении
	sockaddr_in clientInfo;
	ZeroMemory(&clientInfo, sizeof(clientInfo)); // Инициализация структуры clientInfo
	int clientInfo_size = sizeof(clientInfo);
	SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);
	if (ClientConn == INVALID_SOCKET) {
		cout << "Client detected, but can`t connectto a client. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		closesocket(ClientConn);
		WSACleanup();
		return 1;
	}
	else {
		cout << "Connection to a client established" << endl;
		char clientIP[22];
		inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN); // Преобразование IP-адреса подключенного клиента в стандартный стоковый формат
		cout << "Client connected with IP address " << clientIP << endl;
	}
	// Обмен текстовыми данными между серверомм и клиентом. Отключение, если клиент отправляет "exit"
	vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE); // Создание буферов для отправки и получения данных
	short packet_size = 0; // Размер отправляемого/получаемого пакета в байтах

	while (true) {
		packet_size = recv(ClientConn, servBuff.data(), servBuff.size(), 0); // Получение пакета от клиента. Программа ожидает (системная пауза) until receive
		cout << "Client message: " << servBuff.data() << endl;

		cout << "Server message: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		// Проверить прекращение приема-передачи сообщений сервером
		if (clientBuff[0] == 'e' && clientBuff[1] == 'x' && clientBuff[2] == 'i' && clientBuff[3] == 't') {
			shutdown(ClientConn, SD_BOTH);
			closesocket(ServSock);
			closesocket(ClientConn);
			WSACleanup();
			return 1;
		}
		packet_size = send(ClientConn, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can`t send message to Client. Error # " << WSAGetLastError() << endl;
			closesocket(ServSock);
			closesocket(ClientConn);
			WSACleanup();
			return 1;
		}
	}
	closesocket(ServSock);
	closesocket(ClientConn);
	WSACleanup();
	return 0;
}