#pragma once
#include "Global.h"

class ServerMgr
{
private:
	unordered_map<string, UserInfo*> m_mapUser;
	unordered_map<string, Room*> m_mapRoom;
	int m_iID;

public:
	static ServerMgr* GetInst()
	{
		static ServerMgr Inst;
		return &Inst;
	}
	ServerMgr();

public:
	void InsertUser(string& _strKey, SOCKET _hSock);
	Room* InsertRoom();
	UserInfo* FindUserInfo(string& _strKey);
	Room* FindRoom(string& _strKey);
};