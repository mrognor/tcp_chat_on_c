#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string> 

#define MaxMessageLength 256

struct Room
{
	Room(std::string name, std::string password) 
	{
		Name = name; 
		Password = password;
	}
	std::string Name;
	std::string Password;
	std::vector<int> Users;
};

std::vector<SOCKET> Connections;

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

/// Only first element
template <class T>
void RemoveElementFromVectorByName(std::vector<T>& Vector, T Value)
{
	for (auto it = Vector.begin(); it != Vector.end(); it++)
	{
		if (*it == Value)
		{
			Vector.erase(it);
			break;
		}
	}
}

std::vector<Room> Rooms;

void MessageHandler(int UserId, char msg[MaxMessageLength], int* RoomId)
{
	std::vector<std::string> MessageVector = Split(std::string(msg), " ");
	std::string Message;

	if (MessageVector[0] == "!exit")
	{
		RemoveElementFromVectorByName(Rooms[*RoomId].Users, UserId);
		Message = "Your exit room with name: " + Rooms[*RoomId].Name;
		*RoomId = -1;
		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
	}

	if (*RoomId != -1)
	{
		for (int it : Rooms[*RoomId].Users)
		{
			if (UserId != it)
				send(Connections[it], msg, MaxMessageLength, NULL);
		}
		return;
	}

	if (MessageVector[0] == "ls")
	{
		for (int i = 0; i < Rooms.size(); i++)
		{
			Message = Rooms[i].Name;
			send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		}
		return;
	}

	if (MessageVector[0] == "create" && MessageVector.size() >= 3)
	{
		Message = "You are created room: " + MessageVector[1];
		bool IsTakenName = false;
		for (auto it = Rooms.begin(); it != Rooms.end(); it++)
		{
			if ((*it).Name == MessageVector[1])
			{
				Message = "This room name alredy taken";
				IsTakenName = true;
				break;
			}
		}
		if (IsTakenName == false)
		{
			Room r(MessageVector[1], MessageVector[2]);
			Rooms.push_back(r);
		}

		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		return;
	}

	if (MessageVector[0] == "create" && MessageVector.size() < 3)
	{
		Message = "Wrong command. You have to specify room name and password\nCommand usage: create room_name room_password";
		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		return;
	}

	if (MessageVector[0] == "remove" && MessageVector.size() >= 3)
	{
		Message = "You are remove room: " + MessageVector[1];

		bool WasThisRoom = false;
		for (auto it = Rooms.begin(); it != Rooms.end(); it++)
		{
			if ((*it).Name == MessageVector[1])
			{
				WasThisRoom = true;
				if ((*it).Password == MessageVector[2])
				{
					Rooms.erase(it);
					break;
				}
				else 
					Message = "Wrong password";
			}
		}
		
		if (WasThisRoom == false)
			Message = "Wrong name";
		
		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		return;
	}

	if (MessageVector[0] == "remove" && MessageVector.size() < 3)
	{
		Message = "Wrong command. You have to specify room name and password\nCommand usage: remove room_name room_password";
		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		return;
	}

	if (MessageVector[0] == "open" && MessageVector.size() >= 3)
	{
		Message = "Opened room: " + MessageVector[1];
		bool IsCorrectData = false;
		for (int i = 0; i < Rooms.size(); i++)
		{
			if (Rooms[i].Name == MessageVector[1] && Rooms[i].Password == MessageVector[2])
			{
				Rooms[i].Users.push_back(UserId);
				IsCorrectData = true;
				*RoomId = i;
				break;
			}
		}
		if (IsCorrectData == false)
			Message = "Wrong room name or password";
		
		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		return;
	}

	if (MessageVector[0] == "open" && MessageVector.size() < 3)
	{ 
		Message = "Wrong command. You have to specify room name and password\nCommand usage: open room_name room_password";
		send(Connections[UserId], Message.c_str(), MaxMessageLength, NULL);
		return;
	}
}

void ClientHandler(int i)
{
	std::cout << "Client Connected! id: " << i << std::endl;
	char msg[MaxMessageLength] = "Welcome. You are connected to server.";
	send(Connections[i], msg, MaxMessageLength, NULL);

	int ConnectionStatus;
	int RoomId = -1;

	while (true)
	{
		ConnectionStatus = recv(Connections[i], msg, sizeof(msg), NULL);

		if (ConnectionStatus <= 0)
		{
			std::cout << "Client disconnected. id: " << i << std::endl;
			if(RoomId != -1)
				RemoveElementFromVectorByName(Rooms[RoomId].Users, i);
			
			break;
		}

		MessageHandler(i, msg, &RoomId);
	}

	closesocket(i);
	Connections[i] = INVALID_SOCKET;
	if (i == Connections.size() - 1)
		Connections.pop_back();
	std::cout << Connections.size() << std::endl;
	return;
}

int main()
{
	//WSAStartup
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
	{
		std::cout << "Error: Library initialization failure." << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("192.168.1.69");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;

	while (true)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == INVALID_SOCKET)
		{
			std::cout << "Error: Client connection failure." << std::endl;
		}
		else
		{
			bool WasReusedSocket = false;
			for (int i = 0; i < Connections.size(); i++)
			{
				if (Connections[i] == INVALID_SOCKET)
				{
					CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
					Connections[i] = newConnection;
					WasReusedSocket = true;
					std::cout << "Reused socket" << std::endl;
					break;
				}
			}
			if (WasReusedSocket == false)
			{
				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(Connections.size()), NULL, NULL);
				Connections.push_back(newConnection);
			}
		}
	}
}