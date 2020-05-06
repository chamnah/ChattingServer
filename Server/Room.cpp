#include "Room.h"

Room::Room()
{
}

void Room::PushUser(UserInfo* _pUser)
{
	m_vecUser.push_back(_pUser);
}

void Room::BroadCast(LPPER_IO_DATA _ioInfo)
{
	_ioInfo->rwMode = WRITE;
	_ioInfo->iCount = m_vecUser.size();

	for (size_t i = 0; i < _ioInfo->iCount; i++)
	{
		memset(&(_ioInfo->overlap), 0, sizeof(OVERLAPPED));
		WSASend(m_vecUser[i]->hSock,&_ioInfo->wsaBuf,1,NULL,NULL, &_ioInfo->overlap,NULL);
	}
}