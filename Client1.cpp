#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>

#define MaxMessageLength 256
#define IsDebug false

SOCKET Connection = INVALID_SOCKET;

std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString)
{
	std::vector<std::string> ReturnVector;
	int i = 0;
	std::string SplittedString = "";
	while (i < StringToSplit.size())
	{
		if (StringToSplit[i] == SplitterString[0])
		{
			bool IsSplitter = true;
			for (int j = 1; j < SplitterString.size(); j++)
			{
				if (StringToSplit[i + j] != SplitterString[j])
				{
					IsSplitter = false;
					break;
				}
			}
			if (IsSplitter)
			{
				ReturnVector.push_back(SplittedString);
				SplittedString = "";
				i += SplitterString.size();
				continue;
			}
		}
		SplittedString += StringToSplit[i];
		i++;
	}
	ReturnVector.push_back(SplittedString);
	return ReturnVector;
}

void ClientHandler()
{
	char msg[256];
	int ConnectionStatus;
	while (true)
	{
		ConnectionStatus = recv(Connection, msg, MaxMessageLength, NULL);

		if (ConnectionStatus <= 0)
		{
			std::cout << "Server disconnected" << std::endl;
			closesocket(Connection);
			Connection = INVALID_SOCKET;
			break;
		}

		std::cout << msg << std::endl;
	}
}

bool ConnectToServer(std::string ServerAddress, int Port)
{
	//WSAStartup
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
	{
		std::cerr << "Winsock init fail!" << std::endl;
		return false;
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr(ServerAddress.c_str());
	addr.sin_port = htons(Port);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);

	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
	{
		std::cout << "Error: failed connect to server." << std::endl;
		return false;
	}

	//std::cout << "Connected to server" << std::endl;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);
}

int main()
{
	std::string Message;

	std::cout << "Use command help to get information" << std::endl;

	if (IsDebug)
		ConnectToServer("127.0.0.1", 1111);

	while (true)
	{
		getline(std::cin, Message);
		
		if (Message.size() >= MaxMessageLength)
		{
			std::cerr << "Too long message" << std::endl;
			continue;
		}

		std::vector<std::string> MessageVector = Split(Message, " ");

		if (MessageVector[0] == "help")
		{
			std::cout << "Use command connect to connect server" << std::endl;
			std::cout << "Use command disconnect to disconnect server" << std::endl;
			std::cout << "Use command create to create room" << std::endl;
			std::cout << "Use command remove to remove room" << std::endl;
			std::cout << "Use command open to open room" << std::endl;
			std::cout << "Use command !exit to exit room" << std::endl;
			std::cout << "Use command ls to show all rooms" << std::endl;
			continue;
		}
		
		if (MessageVector[0] == "connect" && MessageVector.size() >= 3 )
		{
			if (Connection == INVALID_SOCKET)
			{
				// Address: "127.0.0.1:1111"
				ConnectToServer(MessageVector[1], atoi(MessageVector[2].c_str()));
			}
			else
				std::cout << "You alredy connected to server" << std::endl;
			continue;
		}
		if (MessageVector[0] == "connect" && MessageVector.size() < 3)
		{
			std::cout << "Wrong command. You have to specify server ip address and port" << std::endl;
			std::cout << "Command usage: connect 127.0.0.1 1111" << std::endl;
			continue;
		}

		if (MessageVector[0] == "disconnect")
		{
			closesocket(Connection);
			Connection = INVALID_SOCKET;
			continue;
		}

		send(Connection, Message.c_str(), MaxMessageLength, NULL);
		Sleep(10);
	}
}