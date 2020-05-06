#pragma once

#include "Global.h"

class Room
{
private:
	string m_strID;
	string m_strName;
	vector<UserInfo*> m_vecUser;

public:
	Room();
	void SetID(string& _strID) { m_strID = _strID; }
	string& GetID() { return m_strID; }
	void SetName(string& _strName) { m_strName = _strName; }
	string& GetName() { return m_strName; }
	void PushUser(UserInfo* _pUser);
	void BroadCast(LPPER_IO_DATA _ioInfo);
};