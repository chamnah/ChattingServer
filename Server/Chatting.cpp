#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include "Global.h"
#include <windows.h>
#include "ServerMgr.h"
#include "Room.h"
#include <list>
using std::list;

typedef struct    // socket info
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
	char buffer[BUF_SIZE];

} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct     // buffer info
{
	SOCKET hClntSock;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
} SOCK_DATA;

void SliceText(char* _pOri, string& strFirst, string& strSecond, string& strThird, string& strFour);
void SliceText(char* _pOri,string& strFirst, string& strSecond, string& strThird);
void SliceText(char* _pOri, string& strFirst, string& strSecond);
list<SOCK_DATA> listSock;
HANDLE hMutex;

unsigned int WINAPI ThreadIO(LPVOID CompletionPortIO);

int main(int argc, char* argv[])
{
	WSADATA	wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAdr;
	DWORD recvBytes, i, flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);

	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, ThreadIO, (LPVOID)hComPort, 0, NULL);
	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(9100/*atoi(argv[1])*/);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);

	int iNum = 0;
	hMutex = CreateMutex(NULL, FALSE, NULL);
	while (1)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlap), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;
		ioInfo->iCount = 0;
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf),
			1, &recvBytes, &flags, &(ioInfo->overlap), NULL);
	}

	CloseHandle(hMutex);

	return 0;
}

unsigned int WINAPI ThreadIO(LPVOID pComPort)
{
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1)
	{
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
			(LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ)
		{
			puts("message received!");
			if (bytesTrans == 0)    // EOF 전송 시
			{
				/*ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
				ioInfo->rwMode = WRITE;
				ioInfo->wsaBuf.len = BUF_SIZE;
				ioInfo->wsaBuf.buf = ioInfo->buffer;
				ioInfo->iCount = 0;*/

				//mapSock.find();
				list<SOCK_DATA>::iterator iter = listSock.begin();
				for (; iter != listSock.end();)
				{
					if (sock == iter->hClntSock)
						iter = listSock.erase(iter);
					else
					{
						/*memset(&(ioInfo->overlap), 0, sizeof(OVERLAPPED));
						WSASend(iter->hClntSock, &ioInfo->wsaBuf,1,NULL,NULL, &ioInfo->overlap,NULL);*/
						++iter;
					}
				}


				closesocket(sock);
				free(handleInfo); free(ioInfo);
				continue;
			}

			
			// 처음 로그인 정보
			if (ioInfo->wsaBuf.buf[0] == '0')
			{
				list<SOCK_DATA>::iterator iter = listSock.begin();
				ioInfo->iCount = listSock.size();
				LPPER_IO_DATA ioLog = nullptr;
				int iLength = 0;

				if (ioInfo->iCount != 0)
					ioLog = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));

				for (; iter != listSock.end(); ++iter)
				{
					memset(&(ioInfo->overlap), 0, sizeof(OVERLAPPED));
					ioInfo->wsaBuf.len = bytesTrans;
					ioInfo->rwMode = WRITE;
					ioInfo->buffer[bytesTrans - 1] = '\0';

					WSASend(iter->hClntSock, &(ioInfo->wsaBuf),
						1, NULL, 0, &(ioInfo->overlap), NULL);

					memcpy(&ioLog->buffer[iLength], iter->buffer, iter->wsaBuf.len);
					iLength += iter->wsaBuf.len - 1;
				}
				
				// 로그인한 유저에게 접속한 모든 유저 정보를 보낸다.
				if (ioInfo->iCount != 0)
				{
					memset(&(ioLog->overlap), 0, sizeof(OVERLAPPED));
					ioLog->rwMode = WRITE;

					ioLog->iCount = 1;
					ioLog->wsaBuf.buf = ioLog->buffer;
					ioLog->wsaBuf.len = iLength + 1;
					ioLog->buffer[iLength] = '\0';
					WSASend(sock, &(ioLog->wsaBuf), 1, NULL, 0, &(ioLog->overlap), NULL);
				}

				SOCK_DATA Data;
				Data.hClntSock = sock;
				memcpy(Data.buffer,ioInfo->wsaBuf.buf, bytesTrans);
				Data.wsaBuf.buf = Data.buffer;
				Data.wsaBuf.len = bytesTrans;
				listSock.push_back(Data);
				string strName = Data.wsaBuf.buf + 1;
				ServerMgr::GetInst()->InsertUser(strName, sock);
			}
			// 친구찾기
			else if (ioInfo->wsaBuf.buf[0] == '1')
			{
				string strFind = &ioInfo->wsaBuf.buf[1];
				UserInfo* pUser = ServerMgr::GetInst()->FindUserInfo(strFind);
				memset(&ioInfo->overlap, 0, sizeof(OVERLAPPED));
				ioInfo->iCount = 1;
				ioInfo->rwMode = WRITE;
				ioInfo->wsaBuf.len = 3;
				if (pUser)
				{
					ioInfo->wsaBuf.buf[1] = '1';
					WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlap), NULL);
				}
				else
				{
					ioInfo->wsaBuf.buf[1] = '\0';
					WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlap), NULL);
				}
			}
			// 방이 있는경우
			else if (ioInfo->wsaBuf.buf[0] == '2')
			{
				string strRoom;
				string strFrom;
				string strText;
				string strTo;
				SliceText(ioInfo->buffer, strRoom, strFrom, strTo, strText);

				Room* pRoom = ServerMgr::GetInst()->FindRoom(strRoom);
				if (pRoom)
					pRoom->BroadCast(ioInfo);
			}
			// 방이 없는 경우
			else if (ioInfo->wsaBuf.buf[0] == '3')
			{

				string strTo;
				string strFrom;
				SliceText(ioInfo->buffer, strTo,strFrom);

				UserInfo* pTo = ServerMgr::GetInst()->FindUserInfo(strTo);
				UserInfo* pFrom = ServerMgr::GetInst()->FindUserInfo(strFrom);
				Room* pRoom = ServerMgr::GetInst()->InsertRoom();
				pRoom->PushUser(pTo);
				pRoom->PushUser(pFrom);

				string strSend = "3" + pRoom->GetID() + '/' + strTo + '/' + strFrom;
				strcpy_s(ioInfo->buffer, strSend.c_str());
				ioInfo->wsaBuf.buf = ioInfo->buffer;
				ioInfo->wsaBuf.len = strSend.size();
				pRoom->BroadCast(ioInfo);
			}
			else if (ioInfo->wsaBuf.buf[0] == '4' || ioInfo->wsaBuf.buf[0] == '5')
			{
				string strRoom;
				string strTo;
				string strFrom;
				string strRoomName;
				string strSend;
				SliceText(ioInfo->buffer, strRoom, strTo, strFrom, strRoomName);

				Room* pRoom = ServerMgr::GetInst()->FindRoom(strRoom);
				UserInfo* pUser = ServerMgr::GetInst()->FindUserInfo(strTo);
				if (pRoom && pUser)
				{
					
					if (ioInfo->wsaBuf.buf[0] == '4')
						strSend = "4" + strTo + '/' + pRoom->GetID() + '/' + strRoomName;
					else if (ioInfo->wsaBuf.buf[0] == '5')
						strSend = "5" + strTo + '/' + strFrom + '/' + pRoom->GetID() + '/' + strRoomName;

					strcpy_s(ioInfo->buffer, strSend.c_str());
					ioInfo->wsaBuf.buf = ioInfo->buffer;
					ioInfo->wsaBuf.len = strSend.size();

					pRoom->PushUser(pUser);
					pRoom->BroadCast(ioInfo);
					pRoom->SetName(strRoomName);
				}
			}

			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlap), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf),
				1, NULL, &flags, &(ioInfo->overlap), NULL);
		}
		else if (ioInfo->rwMode == WRITE)
		{
			puts("message sent!");

			WaitForSingleObject(hMutex, INFINITE);
			ioInfo->iCount -= 1;
			ReleaseMutex(hMutex);

			if (ioInfo->iCount <= 0)
			{
				free(ioInfo);
			}
		}
	}
	return 0;
}

void SliceText(char* _pOri, string& strFirst, string& strSecond, string& strThird)
{
	int iNext = 1;
	string strTemp;
	while (true)
	{
		if (_pOri[iNext] == '/')
		{
			++iNext;
			if (strFirst.empty())
			{
				strFirst = strTemp;
				strTemp = "";
			}
			else
			{
				strSecond = strTemp;
				break;
			}
		}
		strTemp += _pOri[iNext];
		++iNext;
	}
	strThird = &_pOri[iNext];
}

void SliceText(char* _pOri, string& strFirst, string& strSecond, string& strThird, string& strFour)
{
	int iNext = 1;
	string strTemp;
	while (true)
	{
		if (_pOri[iNext] == '/')
		{
			++iNext;
			if (strFirst.empty())
			{
				strFirst = strTemp;
				strTemp = "";
			}
			else if (strSecond.empty())
			{
				strSecond = strTemp;
				strTemp = "";
			}
			else
			{
				strThird = strTemp;
				break;
			}
		}
		strTemp += _pOri[iNext];
		++iNext;
	}
	strFour = &_pOri[iNext];
}

void SliceText(char* _pOri, string& strFirst, string& strSecond)
{
	int iNext = 1;
	while (true)
	{
		if (_pOri[iNext] == '/')
			break;
		strFirst += _pOri[iNext];
		++iNext;
	}
	strSecond = &_pOri[iNext + 1];
}