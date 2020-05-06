#include "ServerMgr.h"
#include "Room.h"

ServerMgr::ServerMgr()
	:m_iID(0)
{
}

void ServerMgr::InsertUser(string& _strKey, SOCKET _hSock)
{
	unordered_map<string, UserInfo*>::iterator iter = m_mapUser.find(_strKey);
	if (iter != m_mapUser.end())
		iter->second->hSock = _hSock;
	else
	{
		UserInfo* pUser = new UserInfo;
		pUser->hSock = _hSock;
		m_mapUser.insert(unordered_map<string, UserInfo*>::value_type(_strKey, pUser));
	}
}

Room* ServerMgr::InsertRoom()
{
	char szID[10];
	Room* pRoom = new Room;
	_itoa_s(m_iID, szID,10);
	m_mapRoom.insert(unordered_map<string, Room*>::value_type(szID, pRoom));
	string strTemp = szID;
	pRoom->SetID(strTemp);
	++m_iID;

	return pRoom;
}

UserInfo* ServerMgr::FindUserInfo(string& _strKey)
{
	unordered_map<string, UserInfo*>::iterator iter = m_mapUser.find(_strKey);
	if (iter != m_mapUser.end())
		return iter->second;

	return nullptr;
}

Room* ServerMgr::FindRoom(string& _strKey)
{
	unordered_map<string, Room*>::iterator iter = m_mapRoom.find(_strKey);
	if (iter != m_mapRoom.end())
		return iter->second;

	return nullptr;
}