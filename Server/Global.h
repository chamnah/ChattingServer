#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#pragma comment (lib,"ws2_32.lib")
using std::unordered_map;
using std::string;
using std::vector;

#define BUF_SIZE 100
#define READ	3
#define	WRITE	5

class Room;
struct UserInfo
{
	SOCKET hSock;
	string strName;
	vector<string> vecFriend;
	vector<Room*> vecRoom;
};

typedef struct    // buffer info
{
	OVERLAPPED overlap;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;    // READ or WRITE
	int iCount;
} PER_IO_DATA, * LPPER_IO_DATA;